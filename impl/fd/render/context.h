#pragma once

#include "backend.h"

#if !defined(IMGUI_VERSION) && defined(IMGUI_USER_CONFIG)
#include IMGUI_USER_CONFIG
#endif
#include <imgui_internal.h>
//

#include <Windows.h>

namespace fd
{
struct render_context
{
    ImGuiContext context;
    ImFontAtlas font_atlas;

    render_context();

    render_backend backend;
    HWND window;

    /*void reset_backend();

    void on_window_message();
    void on_resize();
    void on_backend_reset();

    void render();*/

    bool can_render() const;
};

using render_context_ptr = render_context const *;
}