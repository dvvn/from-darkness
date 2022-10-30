module;

#include <fd/assert.h>

#ifdef IMGUI_USER_CONFIG
#include IMGUI_USER_CONFIG
#else
#include <imgui_internal.h>
#endif

#include <windows.h>

module fd.hooks.winapi;
import fd.gui.context;
// import fd.gui.menu;

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;
using namespace hooks;

wndproc::wndproc(HWND hwnd, function_getter target)
    : impl("WinAPI.WndProc")
    , instance(target)
{
    this->def_ = IsWindowUnicode(hwnd) ? DefWindowProcW : DefWindowProcA;
#ifdef _DEBUG
    this->hwnd_ = hwnd;
#endif
}

LRESULT WINAPI wndproc::callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept
{
    FD_ASSERT(window == self->hwnd_);

#define ARGS window, message, w_param, l_param

    // #ifdef IMGUI_DISABLE_DEMO_WINDOWS
    //     if (!gui::menu->visible())
    //         return call_original(ARGS);
    // #endif

    std::pair args = { std::pair(window, message), std::pair(w_param, l_param) };

    switch (gui::context->process_keys(&args))
    {
    case TRUE:
        return TRUE;
    case FALSE:
        return call_original(ARGS);
    default:
        return self->def_(ARGS);
    }
}
