#include <fd/gui/context.h>

#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

#include <d3d9.h>

#include <numeric>
#include <span>

// ReSharper disable once CppInconsistentNaming
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace fd
{
imgui_backup::~imgui_backup()
{
    ImGui::SetAllocatorFunctions(allocator_, deleter_, userData_);
    ImGui::SetCurrentContext(context_);
}

imgui_backup::imgui_backup()
    : context_(ImGui::GetCurrentContext())
{
    ImGui::GetAllocatorFunctions(&allocator_, &deleter_, &userData_);
}

_gui_context::~_gui_context()
{
    if (attached_)
        ImGui_ImplDX9_Shutdown();
    ImGui::Shutdown();
}

_gui_context::_gui_context()
    : context_(&fontAtlas_)
    , focused_(false)
    , attached_(false)
{
    IMGUI_CHECKVERSION();
    ImGui::SetCurrentContext(&context_);
}

bool _gui_context::init(init_data initData)
{
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
    ImGui::SetAllocatorFunctions(
        [](size_t size, void*) { return operator new(size, std::nothrow); },
        [](void* buff, void*) { operator delete(buff, std::nothrow); });
#endif
    ImGui::Initialize();
    focused_ = (GetActiveWindow() == initData.window);
#ifndef _DEBUG
    // todo: disable fallback window
#endif
    if (!initData.storeSettings)
    {
        context_.SettingsHandlers.clear();
        context_.IO.IniFilename = nullptr;
    }
    // ctx_.IO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    if (!ImGui_ImplDX9_Init(initData.backend))
        return false;
    attached_ = true;
    if (!ImGui_ImplWin32_Init(initData.window))
        return false;
    return true;
}

void _gui_context::detach()
{
    attached_ = false;
}

void _gui_context::release_textures()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

#ifdef _DEBUG
// ReSharper disable once CppInconsistentNaming
#define D3D_VALIDATE(_X_) assert(_X_ == D3D_OK)
#else
// ReSharper disable once CppInconsistentNaming
#define D3D_VALIDATE(_X_) _X_
#endif

// ReSharper disable once CppMemberFunctionMayBeConst
bool _gui_context::begin_frame()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

#ifndef IMGUI_HAS_VIEWPORT
    // sets in win32 impl
    auto displaySize = context_.IO.DisplaySize;
    auto minimized   = displaySize.x <= 0 || displaySize.y <= 0;
    if (minimized)
        return false;
#endif

    ImGui::NewFrame();
    return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void _gui_context::end_frame(IDirect3DDevice9* thisPtr)
{
    ImGui::Render();

    D3D_VALIDATE(thisPtr->BeginScene());
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    D3D_VALIDATE(thisPtr->EndScene());
}

struct keys_data
{
    HWND   window;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
};

auto _gui_context::process_keys(void* data) -> keys_return
{
    auto kd = static_cast<keys_data*>(data);
    return process_keys(kd->window, kd->message, kd->wParam, kd->lParam);
}

auto _gui_context::process_keys( //
    HWND   window,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam) -> keys_return
{
#if 0
    if (!can_process_keys())
        return keys_return::native;
#endif

    // update focus
    auto focusChanged = [&]
    {
        switch (message)
        {
        case WM_SETFOCUS:
        {
            focused_ = true;
            return true;
        }
        case WM_KILLFOCUS:
        {
            focused_ = false;
            return true;
        }
        default:
            return false;
        }
    }();

    if (!focused_ && !focusChanged)
        return keys_return::native;

    auto events         = std::span(context_.InputEventsQueue);
    auto oldEventsCount = (events.size());
    auto instant        = ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam) != 0;
    auto eventsAdded    = events.subspan(oldEventsCount);

    if (context_.IO.AppFocusLost)
        return keys_return::native;
    if (instant)
        return keys_return::instant;
    if (!focused_ || (eventsAdded.empty()))
        return keys_return::native;
    return keys_return::def;
}
} // namespace fd