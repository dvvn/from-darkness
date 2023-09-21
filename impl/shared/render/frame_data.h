#pragma once

#include <cstdint>

namespace fd
{
struct basic_render_backend;
struct basic_system_backend;
struct basic_render_context;

struct basic_menu;

struct render_frame_data
{
    basic_render_backend* render_backend;
    basic_system_backend* system_backend;
    basic_render_context* render_context;

    basic_menu* menu;
};
} // namespace fd
