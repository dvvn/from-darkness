#pragma once

#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

#include <fd/core.h>

// ReSharper disable CppInconsistentNaming
struct IDirect3DDevice9;
#define FD_RENDER_BACKEND IDirect3DDevice9

// reserved

//  ReSharper restore CppInconsistentNaming

namespace fd
{
struct basic_render_context : noncopyable
{
    enum class state : uint8_t
    {
        nothing = 0,
        window  = 1 << 0,
        render  = 1 << 1,
    };

  private:
    ImFontAtlas font_atlas_;
    ImGuiContext context_;
    // HWND window_;
    // FD_RENDER_BACKEND *backend_;
    state state_;

  protected:
    void detach();

  public:
    ~basic_render_context();
    basic_render_context(HWND window, FD_RENDER_BACKEND *backend);

    void reset();

    enum class process_result : uint8_t
    {
        idle,
        updated,
        locked
    };

    void process_message(HWND window, UINT message, WPARAM wParam, LPARAM lParam, process_result *result = nullptr);
    bool begin_frame();
    void end_frame(FD_RENDER_BACKEND *backend);

    class frame_holder : public noncopyable
    {
        basic_render_context *ctx_;
        FD_RENDER_BACKEND *backend_;

      public:
        frame_holder(basic_render_context *ctx, FD_RENDER_BACKEND *backend);
        ~frame_holder();

        explicit operator bool() const;
    };

    [[nodiscard]]
    frame_holder new_frame(FD_RENDER_BACKEND *backend);
};
}