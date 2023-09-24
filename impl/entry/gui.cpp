#include "debug/log.h"
#include "functional/bind.h"
#include "functional/function_holder.h"
#include "gui/menu.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"
#include "render/frame.h"
#include "gui/menu_tab.h"

//
#include "string/view.h"

#include <imgui.h>

namespace fd
{
struct dummy_menu_item final : basic_menu_item
{
    string_view name() const override
    {
        return "Dummy";
    }

    void render() const override
    {
        ImGui::TextUnformatted("Test");
    }
};
} // namespace fd

int main(int argc, int* argv) noexcept
{
    (void)argc;
    (void)argv;

#ifdef _DEBUG
    fd::log_activator log_activator;
#endif
    auto const render_context = fd::make_object<fd::render_context>();
    auto const system_backend = fd::make_object<fd::own_win32_backend>();
    auto const render_backend = fd::make_object<fd::own_dx9_backend>();

    fd::function_holder unload_handler([bk = system_backend.get()] {
        bk->close();
    });

    auto const menu = fd::make_object<fd::menu>(&unload_handler);
    fd::dummy_menu_item menu_item;
    fd::menu_items_packed const menu_items(&menu_item);
    fd::render_frame const render_frame{render_backend, system_backend, render_context, menu, &menu_items};

    for (;;)
    {
        system_backend->peek();

        if (system_backend->closed())
            return EXIT_SUCCESS;
        if (system_backend->minimized())
            continue;

        auto const windows_size = system_backend->size();
        render_backend->resize(windows_size.w, windows_size.h);
        render_frame.render();
    }
}