#pragma once

#include "interface.h"

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_context : basic_interface
{
    virtual void begin_frame() = 0;
    virtual void end_frame()   = 0;
    virtual ImDrawData *data() = 0;
};
} // namespace fd