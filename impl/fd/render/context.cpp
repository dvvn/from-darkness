#include "context.h"

namespace fd
{
render_context::render_context()
    : context(&font_atlas)
    , backend(nullptr)
    , window(nullptr)
{
}

bool render_context::can_render() const
{
#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    auto &displaySize = context.IO.DisplaySize;
    if (displaySize.x <= 0 || displaySize.y <= 0)
        return false;
#endif
    return true;
}

}