#pragma once

#include <cstdint>

namespace fd
{
struct basic_render_backend;
struct basic_system_backend;
struct basic_render_context;

struct basic_menu;
struct basic_joined_menu_items;

struct render_frame_data
{
    basic_render_backend *render_backend;
    basic_system_backend *system_backend;
    basic_render_context *render_context;

    basic_menu *menu;
};

enum class render_frame_menu_mode : uint8_t
{
    single,
    multi
};

template <render_frame_menu_mode Mode>
struct render_frame_menu_data;

template <>
struct render_frame_menu_data<render_frame_menu_mode::single> final
{
    basic_joined_menu_items const *items;
};

template <>
struct render_frame_menu_data<render_frame_menu_mode::multi> final
{
    basic_joined_menu_items const **items;
    size_t items_count;

    /*render_frame_menu_data(joined_menu_items const **items, size_t const items_count)
        : items(items)
        , items_count(items_count)
    {
    }*/
};

template <class P>
render_frame_menu_data(P) -> render_frame_menu_data<render_frame_menu_mode::single>;

template <class P>
render_frame_menu_data(P, size_t) -> render_frame_menu_data<render_frame_menu_mode::multi>;

} // namespace fd
