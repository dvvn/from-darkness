#include "basic_context.h"
#include "basic_render_backend.h"
#include "basic_system_backend.h"
#include "frame.h"
#include "functional/bind.h"
#include "gui/basic_menu.h"

#include <algorithm>

namespace fd
{
render_frame_simple::render_frame_simple(
    basic_render_backend *render_backend,
    basic_system_backend *system_backend,
    basic_render_context *render_context,
    basic_menu *menu,
    basic_variables_group **menu_data,
    uint8_t menu_data_length
)
    : render_backend_(render_backend)
    , system_backend_(system_backend)
    , render_context_(render_context)
    , menu_(menu)
    , menu_data_(menu_data)
    , menu_data_length_(menu_data_length)
{
}

void render_frame_simple::render()
{
    system_backend_->new_frame();
    render_backend_->new_frame();
    menu_->new_frame();

    render_context_->begin_frame();
    if (menu_->visible())
    {
        if (menu_->begin_scene())
        {
            std::for_each(menu_data_, menu_data_ + menu_data_length_, bind_front(&basic_menu::render, menu_));
        }
        menu_->end_scene();
    }
    render_context_->end_frame();

    render_backend_->render(render_context_->data());
}

void render_frame_full::render()
{
    if (system_backend_->minimized())
        return;
    render_frame_simple::render();
}
}