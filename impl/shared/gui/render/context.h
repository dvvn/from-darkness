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

    static void begin_frame();
    static void end_frame();
    static ImDrawData* data();
};
} // namespace fd::gui