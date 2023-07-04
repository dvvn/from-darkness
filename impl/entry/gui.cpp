#include "render/backend/own/dx9.h"
#include "render/backend/own/win32.h"
#include "render/context.h"

int main(int argc, int *argv) noexcept
{
    using namespace fd;

    ignore_unused(argc, argv);

    render_context rctx;
    win32_backend_own win32(GetDesktopWindow());
    dx9_backend_own dx9;

    while (auto params = win32.peek())
    {
        if (params->minimized)
            continue;

        dx9.resize(params->w, params->h);

        win32.new_frame();
        dx9.new_frame();

        if (!rctx.begin_scene())
            continue;

        ImGui::ShowDemoWindow();

        rctx.end_scene();

        dx9.render(ImGui::GetDrawData());
    }
}