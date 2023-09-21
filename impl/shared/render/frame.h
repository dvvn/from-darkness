#pragma once
#include "basic_frame.h"
#include "frame_data.h"

#include <utility>

namespace fd
{
#if 0
namespace detail
{
void render(render_frame_data const &data, render_frame_menu_data<render_frame_menu_mode::single> const &menu_data);
void render(render_frame_data const &data, render_frame_menu_data<render_frame_menu_mode::multi> const &menu_data);

void render_if_shown(
    render_frame_data const &data, render_frame_menu_data<render_frame_menu_mode::single> const &menu_data);
void render_if_shown(
    render_frame_data const &data, render_frame_menu_data<render_frame_menu_mode::multi> const &menu_data);
} // namespace detail

template <render_frame_menu_mode Mode>
class render_frame final : public basic_render_frame
{
    render_frame_data data_;
    render_frame_menu_data<Mode> menu_data_;

  public:
    render_frame(render_frame_data data, render_frame_menu_data<Mode> menu_data)
        : data_(std::move(data))
        , menu_data_(std::move(menu_data))
    {
    }

    void render() const override
    {
        detail::render(data_, menu_data_);
    }

    void render_if_shown() const override
    {
        detail::render_if_shown(data_, menu_data_);
    }

    void *native_render() const override
    {
        return data_.render_backend->native();
    }
};

render_frame(render_frame_data, render_frame_menu_data<render_frame_menu_mode::single>)
    -> render_frame<render_frame_menu_mode::single>;
render_frame(render_frame_data, render_frame_menu_data<render_frame_menu_mode::multi>)
    -> render_frame<render_frame_menu_mode::multi>;
#endif
} // namespace fd