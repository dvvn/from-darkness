#pragma once

namespace fd
{
template <class RenderB, class SystemB, class RenderCtx, class Menu>
struct render_frame final
{
    RenderB* render_backend;
    SystemB* system_backend;
    RenderCtx* render_context;
    Menu* menu;

    void operator()() const
    {
        render_backend->new_frame();
        system_backend->new_frame();
        menu->new_frame();

        render_context->begin_frame();
        if (menu->visible())
            menu->render();
        render_context->end_frame();

        render_backend->render(render_context->data());
    }
};
} // namespace fd