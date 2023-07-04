#pragma once

#include "basic_context.h"
#include "noncopyable.h"

#include <imgui_internal.h>

namespace fd
{
class render_context final : public basic_render_context, public noncopyable
{
    ImFontAtlas font_atlas_;
    ImGuiContext context_;

  public:
    ~render_context();
    render_context();

    //bool skip_scene() const override;
    void begin_scene() override;
    void end_scene() override;
};
} // namespace fd