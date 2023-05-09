#include "context.h"

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <utility>

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
render_context::render_context()
    : context(&font_atlas)
    , backend(nullptr)
    , window(nullptr)
{
}

bool render_context::can_render() const
{
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

    return true;
}

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

void *create_render_context(HWND window, void *backend)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&render_context);

#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(custom_allocator);
#elif defined(_DEBUG)
    ImGui::SetAllocatorFunctions(default_allocator);
#endif

    ImGui::Initialize();

    assert(ImGui::FindSettingsHandler("Window"));
    ImGui::RemoveSettingsHandler("Window");

    ImGui::StyleColorsDark();

    if (!ImGui_ImplDX9_Init(static_cast<IDirect3DDevice9 *>(backend)))
        return nullptr;
    if (!ImGui_ImplWin32_Init(window))
        return nullptr;

    return &render_context;
}

void destroy_render_context()
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::Shutdown();
#ifdef _DEBUG
    ImGui::SetCurrentContext(nullptr);
#endif
}
}