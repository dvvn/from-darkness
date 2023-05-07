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
    auto &display_size = context.IO.DisplaySize;
    if (display_size.x <= 0 || display_size.y <= 0)
        return false;
#endif

    /*for (auto w : context.WindowsFocusOrder)
    {
        if (!w->Hidden)
            return true;
        if (w->Active)
            return true;
        if (w->Collapsed)
            return true;
    }*/

    return true;
}

}