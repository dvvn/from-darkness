#pragma once

#include "object.h"

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_context : basic_object
{
    virtual void begin_frame() = 0;
    virtual void end_frame()   = 0;
    virtual ImDrawData *data() = 0;
};
} // namespace fd