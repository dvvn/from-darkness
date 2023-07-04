#pragma once

#include "basic_context.h"
//
#include "type_traits.h"

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

    bool begin_scene() override;
    void end_scene() override;
};
} // namespace fd