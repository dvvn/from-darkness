#pragma once
#include "gui/basic_menu.h"
#include "render/basic_context.h"
#include "render/basic_render_backend.h"
#include "render/basic_system_backend.h"

namespace fd
{
struct render_frame
{
    basic_render_backend* render_backend;
    basic_system_backend* system_backend;
    basic_render_context* render_context;

    basic_menu* menu;
    menu_item_getter const* menu_items;

    void render() const;
};
} // namespace fd