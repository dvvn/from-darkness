#pragma once
#include "tier2/core.h"

namespace FD_TIER2(gui)
{
inline constexpr auto present = //
    []<class RenderB, class SystemB, class RenderCtx, class Menu>(
        RenderB* render_backend, SystemB* system_backend, RenderCtx* render_context, Menu* menu) -> void {
    render_backend->new_frame();
    system_backend->new_frame();
    menu->new_frame();

    render_context->begin_frame();
    if (menu->visible())
        menu->render();
    render_context->end_frame();

    render_backend->render(render_context->data());
};
} // namespace FD_TIER2(gui)