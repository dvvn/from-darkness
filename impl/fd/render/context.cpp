#include "context.h"

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
//[[maybe_unused]] //
// constexpr imgui_alloc_data empty_allocator(nullptr, nullptr);
//[[maybe_unused]] //
// static imgui_alloc_data default_allocator;
//[[maybe_unused]] //
// constexpr imgui_alloc_data(
//    [](size_t size, void *) { return operator new(size, std::nothrow); },
//    [](void *buff, void *) { operator delete(buff, std::nothrow); });
//
// #ifdef _DEBUG
//[[maybe_unused]] //
// static uint8_t alloc_gap = (ImGui::SetAllocatorFunctions(empty_allocator), 1);
// #endif

template <class>
struct d3d_backend
{
    static auto unreachable(...)
    {
        std::unreachable();
    }

    static auto empty()
    {
    }

    static constexpr auto init      = unreachable;
    static constexpr auto shutdown  = empty;
    static constexpr auto reset     = unreachable;
    static constexpr auto new_frame = unreachable;
    static constexpr auto render    = unreachable;
};

template <>
struct d3d_backend<IDirect3DDevice9 *>
{
    static constexpr auto init      = ImGui_ImplDX9_Init;
    static constexpr auto shutdown  = ImGui_ImplDX9_Shutdown;
    static constexpr auto reset     = ImGui_ImplDX9_InvalidateDeviceObjects;
    static constexpr auto new_frame = ImGui_ImplDX9_NewFrame;

    static void render(IDirect3DDevice9 *backend)
    {
        (void)backend->BeginScene();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        (void)backend->EndScene();
    }
};

#define VISIT_BACKEND(...) std::visit([]<class T>(T obj) { d3d_backend<T>::__VA_ARGS__; }, backend_);

render_context::~render_context()
{
    VISIT_BACKEND(shutdown());
    if (window_)
        ImGui_ImplWin32_Shutdown();

    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

render_context::render_context()
    : context(&font_atlas)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&context);

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(
        [](size_t size, void *) { return operator new(size, std::nothrow); },
        [](void *buff, void *) { operator delete(buff, std::nothrow); });
#endif

    ImGui::Initialize();

    /*assert(ImGui::FindSettingsHandler("Window"));
   ImGui::RemoveSettingsHandler("Window");*/
    context.SettingsHandlers.clear();
    context.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();
}

bool render_context::init(HWND window, IDirect3DDevice9 *backend) noexcept
{
    if (!ImGui_ImplDX9_Init(backend))
        return false;
    backend_ = backend;
    if (!ImGui_ImplWin32_Init(window))
        return false;
    window_ = window;
    return true;
}

void render_context::detach()
{
    backend_.emplace<std::monostate>();
}

void render_context::reset()
{
    VISIT_BACKEND(reset());
}

void render_context::process_message(HWND window, UINT message, WPARAM wParam, LPARAM lParam, process_result *result)
{
    assert(window_ == window);

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

    auto &events       = context.InputEventsQueue;
    auto events_stored = events.size();

    if (ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0)
        *result = process_result::locked;
    else if (events_stored != events.size())
        *result = process_result::updated;
    else
        *result = process_result::idle;
}

bool render_context::begin_frame()
{
    VISIT_BACKEND(new_frame());
    ImGui_ImplWin32_NewFrame();

#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    auto &display_size = context.IO.DisplaySize;
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

void render_context::end_frame()
{
    ImGui::Render();
    VISIT_BACKEND(render(obj));
}

render_context::render_frame_holder::render_frame_holder(render_context *ctx)
{
    if (ctx->begin_frame())
        ctx_ = ctx;
}

render_context::render_frame_holder::~render_frame_holder()
{
    if (ctx_)
        ctx_->end_frame();
}

render_context::render_frame_holder::operator bool() const
{
    return ctx_ != nullptr;
}

auto render_context::render_frame() -> render_frame_holder
{
    return this;
}
} // namespace fd