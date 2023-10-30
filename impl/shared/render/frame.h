#pragma once

namespace fd
{
inline constexpr auto render_frame =                               //
    []<class RenderB, class SystemB, class RenderCtx, class Menu>( //
        RenderB* render_backend, SystemB* system_backend, RenderCtx* render_context, Menu* menu) {
        render_backend->new_frame();
        system_backend->new_frame();
        menu->new_frame();

        render_context->begin_frame();
        if (menu->visible())
            menu->render();
        render_context->end_frame();

        render_backend->render(render_context->data());
    };
} // namespace fd