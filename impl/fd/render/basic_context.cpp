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
template <class>
struct select_backend;

template <>
struct select_backend<IDirect3DDevice9>
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

static auto operator&(basic_render_context::state l, basic_render_context::state r)
{
    using u_t = std::underlying_type_t<basic_render_context::state>;
    return static_cast<u_t>(l) & static_cast<u_t>(r);
}

static basic_render_context::state &operator|=(basic_render_context::state &l, basic_render_context::state r)
{
    using u_t                  = std::underlying_type_t<basic_render_context::state>;
    reinterpret_cast<u_t &>(l) = static_cast<u_t>(l) | static_cast<u_t>(r);
    return l;
}

basic_render_context::~basic_render_context()
{
    if (state_ & state::render)
        select_backend<FD_RENDER_BACKEND>::shutdown();
    if (state_ & state::window)
        ImGui_ImplWin32_Shutdown();

    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

basic_render_context::basic_render_context(HWND window, FD_RENDER_BACKEND *backend)
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

    if (!select_backend<FD_RENDER_BACKEND>::init(backend))
        throw runtime_error("Unable to init render backend!");
    state_ |= state::render;
    if (!ImGui_ImplWin32_Init(window))
        throw runtime_error("Unable to init Win32 backend!");
}

void basic_render_context::detach()
{
    state_ = state::nothing;
}

void basic_render_context::reset()
{
    ignore_unused(this);
    select_backend<FD_RENDER_BACKEND>::reset();
}

void basic_render_context::process_message(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    process_result *result)
{
    // reserved, WIP
    auto process = true;

    if (!process)
    {
        if (result)
            *result = process_result::idle;
        return;
    }

    if (!result)
    {
        ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam);
        return;
    }

    auto &events       = context_.InputEventsQueue;
    auto events_stored = events.size();

    if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0)
        *result = process_result::locked;
    else if (events_stored != events.size())
        *result = process_result::updated;
    else
        *result = process_result::idle;
}

bool basic_render_context::begin_frame()
{
    select_backend<FD_RENDER_BACKEND>::new_frame();
    ImGui_ImplWin32_NewFrame();

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

void basic_render_context::end_frame(FD_RENDER_BACKEND *backend)
{
    ignore_unused(this);
    ImGui::Render();
    select_backend<FD_RENDER_BACKEND>::render(backend, ImGui::GetDrawData());
}

basic_render_context::frame_holder::frame_holder(basic_render_context *ctx, FD_RENDER_BACKEND *backend)
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

basic_render_context::frame_holder::~frame_holder()
{
    if (ctx_)
        ctx_->end_frame(backend_);
}

basic_render_context::frame_holder::operator bool() const
{
    return ctx_ != nullptr;
}

auto basic_render_context::new_frame(FD_RENDER_BACKEND *backend) -> frame_holder
{
    return {this, backend};
}
} // namespace fd