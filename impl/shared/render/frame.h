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

    void render() const
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
};
} // namespace fd