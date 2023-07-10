#include "debug/log.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"

int main(int argc, int *argv) noexcept
{
    (void)argc;
    (void)argv;

#ifdef _DEBUG
    fd::log_activator log_activator;
#endif

    auto render = fd::make_interface<fd::render_context>();
    auto win32  = fd::make_interface<fd::own_win32_backend>();
    auto dx9    = fd::make_interface<fd::own_dx9_backend>();

    for (;;)
    {
        win32->peek();

        if (win32->closed())
            return EXIT_SUCCESS;
        if (win32->minimized())
            continue;

        auto windows_size = win32->size();
        dx9->resize(windows_size.w, windows_size.h);

        win32->new_frame();
        dx9->new_frame();

        render->begin_scene();
        {
            // ImGui::ShowDemoWindow();
        }
        render->end_scene();

        dx9->render(render->data());
    }
}