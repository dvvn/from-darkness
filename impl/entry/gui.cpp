#include "debug/log.h"
#include "gui/menu.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"
#include "render/frame.h"
#include "vars/sample.h"

int main(int argc, int *argv) noexcept
{
    (void)argc;
    (void)argv;

#ifdef _DEBUG
    fd::log_activator log_activator;
#endif

    auto const menu           = fd::make_object<fd::menu>();
    auto const vars_sample    = fd::make_object<fd::vars_sample>();
    auto const render_context = fd::make_object<fd::render_context>();
    auto const system_backend = fd::make_object<fd::own_win32_backend>();
    auto const render_backend = fd::make_object<fd::own_dx9_backend>();

    auto vars = join(vars_sample);

    auto const render_frame = fd::make_object<fd::render_frame_simple>(
        render_backend, system_backend, //
        render_context,                 //
        menu, data(vars), size(vars));

    for (;;)
    {
        system_backend->peek();

        if (system_backend->closed())
            return EXIT_SUCCESS;
        if (system_backend->minimized())
            continue;

        auto const windows_size = system_backend->size();
        render_backend->resize(windows_size.w, windows_size.h);

        render_frame->render();
    }
}