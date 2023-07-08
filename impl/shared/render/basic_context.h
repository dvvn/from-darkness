#pragma once

#include "interface.h"

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_context : basic_interface
{
    virtual void begin_scene() = 0;
    virtual void end_scene()   = 0;
    virtual ImDrawData *data() = 0;
};
} // namespace fd