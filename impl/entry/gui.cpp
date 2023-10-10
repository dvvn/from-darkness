#include "debug/log.h"
#include "functional/bind.h"
#include "gui/menu.h"
#include "gui/menu_tab.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"
#include "render/frame.h"
#include "string/view.h"

namespace fd
{
struct dummy_menu_item final
{
    /*string_view name() const
    {
        return "Dummy";
    }*/

    void operator()() const
    {
        ImGui::TextUnformatted("Result");
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

    fd::render_context render_context;
    fd::own_win32_backend system_backend;
    fd::win32_backend_info system_backend_info;
    system_backend.fill(&system_backend_info);
    fd::own_dx9_backend render_backend(system_backend_info.id);
    auto menu_item = fd::dummy_menu_item();
    fd::menu menu(
        fd::menu_items_packed(fd::menu_tab(fd::menu_tab_item("Tab1", fd::menu_items_packed(&menu_item)))),
        bind(&fd::own_win32_backend::close, &system_backend));

    fd::render_frame const render_frame(&render_backend, &system_backend, &render_context, &menu);

    while (system_backend.update())
    {
        if (system_backend_info.minimized())
            continue;
        auto const windows_size = system_backend_info.size();
        render_backend.resize(windows_size.w, windows_size.h);
        render_frame.render();
    }

    return EXIT_SUCCESS;
}