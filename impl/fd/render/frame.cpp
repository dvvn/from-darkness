#include "frame.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
//

#include <d3d9.h>

namespace fd
{
static void write_frame(render_backend backend)
{
    ImGui::Render();

    (void)backend->BeginScene();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    (void)backend->EndScene();
}

render_frame::render_frame(render_context_ptr ctx)
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    if (ctx->can_render())
        ImGui::NewFrame();
    else
        ctx = nullptr;

    ctx_ = ctx;
}

render_frame::~render_frame()
{
    if (ctx_)
        write_frame(ctx_->backend);
}

render_frame::operator bool() const
{
    return ctx_ != nullptr;
}

void render_frame::write()
{
    write_frame(ctx_->backend);
    ctx_ = nullptr;
}
}