#pragma once

#include <type_traits>

namespace fd
{
struct basic_render_backend;
struct basic_system_backend;
struct basic_render_context;

struct basic_menu;
struct basic_variables_group;



struct render_frame
{
    basic_render_backend *render_backend;
    basic_system_backend *system_backend;
    basic_render_context *render_context;

    basic_menu *menu;
    basic_variables_group **menu_data;
    uint8_t menu_data_length;


    void render() const;
};

} // namespace fd