#pragma once

namespace fd
{
template <class RenderB, class SystemB, class RenderContext, class Menu>
struct render_frame final
{
    RenderB* render_backend;
    SystemB* system_backend;
    RenderContext* render_context;
    Menu* menu;

    void render() const
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