#pragma once

#include "object.h"

namespace fd
{
struct basic_render_frame : basic_object
{
    virtual void render() = 0;
};
} // namespace fd