#pragma once

#include <fd/core.h>

#include <imgui_internal.h>

#include <Windows.h>

// ReSharper disable CppInconsistentNaming
struct IDirect3DDevice9;
#define FD_RENDER_BACKEND IDirect3DDevice9

// reserved

//  ReSharper restore CppInconsistentNaming

namespace fd
{
class render_context : public noncopyable
{
    ImFontAtlas font_atlas_;
    ImGuiContext context_;
    HWND window_;
    FD_RENDER_BACKEND *backend_;

  public:
    ~render_context();
    render_context();

    bool init(HWND window, FD_RENDER_BACKEND *backend) noexcept;

    void detach();
    void reset();

    enum class process_result : uint8_t
    {
        idle,
        updated,
        locked
    };

    void process_message(HWND window, UINT message, WPARAM wParam, LPARAM lParam, process_result *result = nullptr);
    bool begin_frame();
    void end_frame();

    class frame_holder : public noncopyable
    {
        render_context *ctx_;

      public:
        frame_holder(render_context *ctx);
        ~frame_holder();

        explicit operator bool() const;
    };

    [[nodiscard]]
    frame_holder new_frame();
};

}