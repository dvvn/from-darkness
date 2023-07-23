#include "debug/log.h"
#include "functional/bind.h"
#include "functional/function_holder.h"
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

    auto const render_context = fd::make_object<fd::render_context>();
    auto const system_backend = fd::make_object<fd::own_win32_backend>();
    auto const render_backend = fd::make_object<fd::own_dx9_backend>();

    fd::function_holder unload_handler(bind(&fd::basic_own_win32_backend::close, system_backend));

    auto const vars_sample = fd::make_object<fd::vars_sample>();
    auto const menu        = fd::make_object<fd::menu>(&unload_handler);

    auto vars = join(vars_sample);

    fd::render_frame const frame(
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

        frame.render();
    }
}