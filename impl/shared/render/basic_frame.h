#pragma once

#include "interface.h"

namespace fd
{
struct basic_render_frame : basic_interface
{
    virtual void render() = 0;
};
} // namespace fd