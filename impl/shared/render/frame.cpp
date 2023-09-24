#include "frame.h"

namespace fd
{
void render_frame::render() const
{
    system_backend->new_frame();
    render_backend->new_frame();
    menu->new_frame();

    render_context->begin_frame();
    if (menu->visible())
    {
        if (menu->begin_scene())
        {
            menu->render(menu_items);
        }
        menu->end_scene();
    }
    render_context->end_frame();

    render_backend->render(render_context->data());
}
}