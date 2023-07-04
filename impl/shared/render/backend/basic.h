#pragma once

#include "base.h"

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_backend : basic_backend
{
    virtual void render(ImDrawData *draw_data) = 0;
    virtual void reset()                       = 0;
};
}