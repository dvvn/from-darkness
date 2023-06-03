#include "context.h"

#include <fd/lazy_invoke.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

#include <Windows.h>
#include <d3d9.h>

#include <utility>

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

struct imgui_alloc_data
{
    ImGuiMemAllocFunc alloc_func;
    ImGuiMemFreeFunc free_func;
    void *user_data;

    imgui_alloc_data()
    {
        ImGui::GetAllocatorFunctions(&alloc_func, &free_func, &user_data);
    }

    constexpr imgui_alloc_data(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void *user_data = nullptr)
        : alloc_func(alloc_func)
        , free_func(free_func)
        , user_data(user_data)
    {
    }
};

// ReSharper disable once CppInconsistentNaming
namespace ImGui
{
// ReSharper disable once CppInconsistentNaming
static void SetAllocatorFunctions(imgui_alloc_data const &data)
{
    SetAllocatorFunctions(data.alloc_func, data.free_func, data.user_data);
}
} // namespace ImGui

namespace fd
{
[[maybe_unused]] //
constexpr imgui_alloc_data empty_allocator(nullptr, nullptr);
[[maybe_unused]] //
static imgui_alloc_data default_allocator;
[[maybe_unused]] //
constexpr imgui_alloc_data custom_allocator(
    [](size_t size, void *) { return operator new(size, std::nothrow); },
    [](void *buff, void *) { operator delete(buff, std::nothrow); });

#ifdef _DEBUG
[[maybe_unused]] //
static uint8_t alloc_gap = (ImGui::SetAllocatorFunctions(empty_allocator), 1);
#endif

static ImFontAtlas font_atlas;
static ImGuiContext render_context(&font_atlas);
static void *render_backend = nullptr;

void *create_render_context(void *window, void *backend) noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&render_context);

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(custom_allocator);
#elif defined(_DEBUG)
    ImGui::SetAllocatorFunctions(default_allocator);
#endif

    ImGui::Initialize();
    invoke_on_destruct shutdown = ImGui::Shutdown;

    /*assert(ImGui::FindSettingsHandler("Window"));
    ImGui::RemoveSettingsHandler("Window");*/
    render_context.SettingsHandlers.clear();
    render_context.IO.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplDX9_Init(static_cast<IDirect3DDevice9 *>(backend)))
        return nullptr;
    invoke_on_destruct shutdown_backend = ImGui_ImplDX9_Shutdown;

    if (!ImGui_ImplWin32_Init(window))
        return nullptr;

    shutdown         = nullptr;
    shutdown_backend = nullptr;

    render_backend = backend;

    (void)shutdown;
    (void)shutdown_backend;

    return &render_context;
}

void destroy_render_context() noexcept
{
    if (render_backend)
        ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}

void render_backend_detach()
{
    render_backend = nullptr;
}

static bool can_render()
{
#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    auto &display_size = render_context.IO.DisplaySize;
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

    return true;
}

void process_render_message(void *window, size_t message, size_t wParam, size_t lParam, render_message_result *result)
{
    // reserved, WIP
    auto process = true;

    if (!process)
    {
        if (result)
            *result = render_message_result::idle;
        return;
    }

    if (!result)
    {
        ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(window), message, wParam, lParam);
        return;
    }

    auto &events       = render_context.InputEventsQueue;
    auto events_stored = events.size();

    if (ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(window), message, wParam, lParam) != 0)
        *result = render_message_result::locked;
    else if (events_stored != events.size())
        *result = render_message_result::updated;
    else
        *result = render_message_result::idle;
}

void reset_render_context()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

bool begin_render_frame()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    auto ok = can_render();
    if (ok)
        ImGui::NewFrame();
    return ok;
}

void end_render_frame()
{
    ImGui::Render();

    auto backend = static_cast<IDirect3DDevice9 *>(render_backend);

    (void)backend->BeginScene();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    (void)backend->EndScene();
}

render_frame::render_frame()
{
    valid_ = begin_render_frame();
}

render_frame::~render_frame()
{
    if (!valid_)
        return;

    end_render_frame();
}

render_frame::operator bool() const
{
    return valid_;
}
} // namespace fd