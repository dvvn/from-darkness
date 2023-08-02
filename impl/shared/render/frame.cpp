#include "basic_context.h"
#include "basic_render_backend.h"
#include "basic_system_backend.h"
#include "frame.h"
#include "functional/bind.h"
#include "gui/basic_menu.h"

#include <algorithm>
#include <cassert>

namespace fd::detail
{
using enum render_frame_menu_mode;

static void render_menu(render_frame_data const &data, render_frame_menu_data<single> const &menu_data)
{
    data.menu->render(menu_data.items);
}

static void render_menu(render_frame_data const &data, render_frame_menu_data<multi> const &menu_data)
{
    for (size_t i = 0; i != menu_data.items_count; ++i)
        data.menu->render(menu_data.items[i]);
}

template <render_frame_menu_mode Mode>
static void render_impl(render_frame_data const &data, render_frame_menu_data<Mode> const &menu_data)
{
    assert(!data.system_backend->minimized());

    data.system_backend->new_frame();
    data.render_backend->new_frame();
    data.menu->new_frame();

    data.render_context->begin_frame();
    if (data.menu->visible())
    {
        if (data.menu->begin_scene())
        {
            render_menu(data, menu_data);
        }
        data.menu->end_scene();
    }
    data.render_context->end_frame();

    data.render_backend->render(data.render_context->data());
}

void render(render_frame_data const &data, render_frame_menu_data<single> const &menu_data)
{
    render_impl<single>(data, menu_data);
}

void render(render_frame_data const &data, render_frame_menu_data<multi> const &menu_data)
{
    render_impl<multi>(data, menu_data);
}

void render_if_shown(render_frame_data const &data, render_frame_menu_data<single> const &menu_data)
{
    if (!data.system_backend->minimized())
        render(data, menu_data);
}

void render_if_shown(render_frame_data const &data, render_frame_menu_data<multi> const &menu_data)
{
    if (!data.system_backend->minimized())
        render(data, menu_data);
}
} // namespace fd::detail
