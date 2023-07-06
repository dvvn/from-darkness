#include "debug/log.h"
#include "functional/ignore.h"
#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"

int main(int argc, int *argv) noexcept
{
    fd::ignore_unused(argc, argv);

#ifdef _DEBUG
    fd::log_activator log_activator;
#endif

    fd::render_context rctx;
    fd::win32_backend_own win32(GetDesktopWindow());
    fd::dx9_backend_own dx9;

    while (auto params = win32.peek())
    {
        if (params->minimized)
            continue;

        dx9.resize(params->w, params->h);

        win32.new_frame();
        dx9.new_frame();

        rctx.begin_scene();
        {
            ImGui::ShowDemoWindow();
        }
        rctx.end_scene();

        dx9.render(rctx.data());
    }
}