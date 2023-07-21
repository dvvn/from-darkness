#pragma once

#include "object.h"

namespace fd
{
struct basic_variables_group;

struct basic_menu : basic_object
{
    virtual void toggle()                             = 0;
    virtual void new_frame()                          = 0;
    virtual bool visible() const                      = 0;
    virtual bool begin_scene()                        = 0;
    virtual void render(basic_variables_group *group) = 0;
    virtual void end_scene()                          = 0;
};
} // namespace fd