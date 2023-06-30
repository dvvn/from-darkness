#include "basic_context.h"

#include "fd/tool/exception.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

#include <utility>

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace fd
{
struct render_backend
{
    static constexpr auto init      = ImGui_ImplDX9_Init;
    static constexpr auto shutdown  = ImGui_ImplDX9_Shutdown;
    static constexpr auto reset     = ImGui_ImplDX9_InvalidateDeviceObjects;
    static constexpr auto new_frame = ImGui_ImplDX9_NewFrame;

    static void render(IDirect3DDevice9 *backend, ImDrawData *draw_data)
    {
        (void)backend->BeginScene();
        ImGui_ImplDX9_RenderDrawData(draw_data);
        (void)backend->EndScene();
    }
};

struct system_backend
{
    static constexpr auto init      = ImGui_ImplWin32_Init;
    static constexpr auto shutdown  = ImGui_ImplWin32_Shutdown;
    static constexpr auto new_frame = ImGui_ImplWin32_NewFrame;
    static constexpr auto update    = ImGui_ImplWin32_WndProcHandler;
};

static auto operator&(basic_render_context::init_state l, basic_render_context::init_state r)
{
    using u_t = std::underlying_type_t<basic_render_context::init_state>;
    return static_cast<u_t>(l) & static_cast<u_t>(r);
}

static basic_render_context::init_state &operator|=(
    basic_render_context::init_state &l,
    basic_render_context::init_state r)
{
    using u_t                  = std::underlying_type_t<basic_render_context::init_state>;
    reinterpret_cast<u_t &>(l) = static_cast<u_t>(l) | static_cast<u_t>(r);
    return l;
}

basic_render_context::~basic_render_context()
{
    if (state_ & init_state::render)
        render_backend::shutdown();
    if (state_ & init_state::system)
        system_backend::shutdown();

    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

basic_render_context::basic_render_context(HWND window, native_render_backend backend)
    : context_(&font_atlas_)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&context_);

#if defined(_DEBUG) || defined(IMGUI_DISABLE_DEFAULT_ALLOCATORS)
    ImGui::SetAllocatorFunctions(
        [](size_t size, void *) { return operator new(size, std::nothrow); },
        [](void *buff, void *) { operator delete(buff, std::nothrow); });
#endif

    ImGui::Initialize();

    /*assert(ImGui::FindSettingsHandler("Window"));
   ImGui::RemoveSettingsHandler("Window");*/
    context_.SettingsHandlers.clear();
    context_.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    if (!render_backend::init(backend))
        throw runtime_error("Unable to init render backend!");
    state_ |= init_state::render;
    if (!system_backend::init(window))
        throw runtime_error("Unable to init Win32 backend!");
}

void basic_render_context::detach()
{
    state_ = init_state::nothing;
}

void basic_render_context::reset()
{
    ignore_unused(this);
    render_backend::reset();
}

void basic_render_context::process_message(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    processed_message *result)
{
    // reserved, WIP
    auto process = true;

    if (!process)
    {
        if (result)
            *result = processed_message::idle;
        return;
    }

    if (!result)
    {
        system_backend::update(window, message, wParam, lParam);
        return;
    }

    auto &events       = context_.InputEventsQueue;
    auto events_stored = events.size();

    if (system_backend::update(window, message, wParam, lParam) != 0)
        *result = processed_message::locked;
    else if (events_stored != events.size())
        *result = processed_message::updated;
    else
        *result = processed_message::idle;
}

bool basic_render_context::begin_frame()
{
    render_backend::new_frame();
    system_backend::new_frame();

#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    auto &display_size = context_.IO.DisplaySize;
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

    ImGui::NewFrame();
    return true;
}

void basic_render_context::end_frame(native_render_backend backend)
{
    ignore_unused(this);
    ImGui::Render();
    render_backend::render(backend, ImGui::GetDrawData());
}

using frame_holder = basic_render_context::frame_holder;

frame_holder::frame_holder(basic_render_context *ctx, native_render_backend backend)
{
    if (!ctx->begin_frame())
    {
        ctx_ = nullptr;
    }
    else
    {
        ctx_     = ctx;
        backend_ = backend;
    }
}

frame_holder::~frame_holder()
{
    if (ctx_)
        ctx_->end_frame(backend_);
}

frame_holder::operator bool() const
{
    return ctx_ != nullptr;
}

auto basic_render_context::new_frame(native_render_backend backend) -> frame_holder
{
    return {this, backend};
}
} // namespace fd