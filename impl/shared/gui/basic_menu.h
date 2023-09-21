#pragma once

#include "object.h"

namespace fd
{
template <typename Ret, typename... Args>
class basic_function;

struct basic_menu_item;
using menu_item_getter = basic_function<basic_menu_item*>;

struct basic_menu : basic_object
{
    virtual void toggle()                         = 0;
    virtual void new_frame()                      = 0;
    virtual bool visible() const                  = 0;
    virtual bool begin_scene()                    = 0;
    virtual void render(basic_menu_item* item)    = 0;
    virtual void render(menu_item_getter* getter) = 0;
    virtual void end_scene()                      = 0;
};
} // namespace fd