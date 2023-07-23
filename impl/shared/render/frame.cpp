#include "basic_context.h"
#include "basic_render_backend.h"
#include "basic_system_backend.h"
#include "frame.h"
#include "functional/bind.h"
#include "gui/basic_menu.h"

#include <algorithm>
#include <cassert>

namespace fd
{
void render_frame::render() const
{
    assert(!system_backend->minimized());

    system_backend->new_frame();
    render_backend->new_frame();
    menu->new_frame();

    render_context->begin_frame();
    if (menu->visible())
    {
        if (menu->begin_scene())
        {
            std::for_each(
                menu_data, menu_data + menu_data_length, //
                bind_front(&basic_menu::render, menu));
        }
        menu->end_scene();
    }
    render_context->end_frame();

    render_backend->render(render_context->data());
}
}