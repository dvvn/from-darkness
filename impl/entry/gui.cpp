#include "debug/log.h"
#include "gui/menu.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"
#include "vars/sample.h"

int main(int argc, int *argv) noexcept
{
    (void)argc;
    (void)argv;

#ifdef _DEBUG
    fd::log_activator log_activator;
#endif

    auto render_context = fd::make_interface<fd::render_context>();
    auto system_backend = fd::make_interface<fd::own_win32_backend>();
    auto render_backend = fd::make_interface<fd::own_dx9_backend>();

    auto menu = fd::make_interface<fd::menu>();

    auto vars_sample = fd::make_interface<fd::vars_sample>();

    for (;;)
    {
        system_backend->peek();

        if (system_backend->closed())
            return EXIT_SUCCESS;
        if (system_backend->minimized())
            continue;

        auto windows_size = system_backend->size();
        render_backend->resize(windows_size.w, windows_size.h);

        menu->new_frame();
        system_backend->new_frame();
        render_backend->new_frame();

        render_context->begin_scene();
        if (menu->visible())
        {
            if (menu->begin_scene())
            {
                menu->render(&vars_sample);
            }
            menu->end_scene();
        }
        render_context->end_scene();

        render_backend->render(render_context->data());
    }
}