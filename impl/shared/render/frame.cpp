#include "basic_context.h"
#include "basic_render_backend.h"
#include "basic_system_backend.h"
#include "frame.h"
#include "functional/bind.h"
#include "gui/basic_menu.h"

#include <algorithm>

namespace fd
{
class render_frame_simple : public basic_render_frame
{
    basic_render_backend *render_backend_;
    basic_system_backend *system_backend_;
    basic_render_context *render_context_;

    basic_menu *menu_;
    basic_variables_group **menu_data_;
    uint8_t menu_data_length_;

  public:
    render_frame_simple(
        basic_render_backend *render_backend, basic_system_backend *system_backend,
        basic_render_context *render_context,
        basic_menu *menu, //
        basic_variables_group **menu_data, size_t const menu_data_length)
        : render_backend_(render_backend)
        , system_backend_(system_backend)
        , render_context_(render_context)
        , menu_(menu)
        , menu_data_(menu_data)
        , menu_data_length_(menu_data_length)
    {
    }

    bool skip_render() const
    {
        return system_backend_->minimized();
    }

    void render() override
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
};

struct render_frame_full final : render_frame_simple
{
    using render_frame_simple::render_frame_simple;

    void render() override
    {
        if (skip_render())
            return;
        render_frame_simple::render();
    }
};

FD_INTERFACE_IMPL(render_frame_simple);
FD_INTERFACE_IMPL(render_frame_full);
}