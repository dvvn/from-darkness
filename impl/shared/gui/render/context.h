#pragma once

#include "noncopyable.h"

#include <imgui_internal.h>

namespace fd::gui
{
class render_context final : public noncopyable
{
    ImFontAtlas font_atlas_;
    ImGuiContext context_;

  public:
    ~render_context();
    render_context();

#if 0
//this method sucks
bool skip_scene() const
{
#ifdef IMGUI_HAS_VIEWPORT
    return false;
#else
    // sets in win32 impl
    auto &display_size = context_.IO.DisplaySize;
    return static_cast<size_t>(display_size.x) != 0 && static_cast<size_t>(display_size.y) != 0;
#endif
}
#endif

    void begin_frame();
    void end_frame();
    ImDrawData* data();
};
} // namespace fd