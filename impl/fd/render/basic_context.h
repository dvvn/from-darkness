#pragma once

#include "native.h"

#include "fd/core.h"

#include <imgui_internal.h>

#include <Windows.h>

namespace fd
{
struct basic_render_context : noncopyable
{
    enum class init_state : uint8_t
    {
        nothing = 0,
        system  = 1 << 0,
        render  = 1 << 1,
    };

    enum class processed_message : uint8_t
    {
        idle,
        updated,
        locked
    };

    class frame_holder : public noncopyable
    {
        basic_render_context *ctx_;
        native_render_backend backend_;

      public:
        frame_holder(basic_render_context *ctx, native_render_backend backend);
        ~frame_holder();

        explicit operator bool() const;
    };

  private:
    ImFontAtlas font_atlas_;
    ImGuiContext context_;
    init_state state_;

  protected:
    void detach();

  public:
    ~basic_render_context();
    basic_render_context(HWND window, native_render_backend backend);

    void reset();

    void process_message(HWND window, UINT message, WPARAM wParam, LPARAM lParam, processed_message *result = nullptr);
    bool begin_frame();
    void end_frame(native_render_backend backend);

    [[nodiscard]]
    frame_holder new_frame(native_render_backend backend);
};
} // namespace fd