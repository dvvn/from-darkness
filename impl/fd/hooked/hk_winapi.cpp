#include <fd/assert.h>
#include <fd/gui/context.h>
#include <fd/gui/menu.h>
#include <fd/hooked/hk_winapi.h>

#ifdef IMGUI_USER_CONFIG
#include IMGUI_USER_CONFIG
#else
#include <imgui_internal.h>
#endif

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;
using namespace hooked;

wndproc::wndproc(HWND hwnd, function_getter target)
    : hook_impl("WinAPI.WndProc")
    , hook_instance(target)
{
    this->def_  = IsWindowUnicode(hwnd) ? DefWindowProcW : DefWindowProcA;
    this->hwnd_ = hwnd;
}

LRESULT WINAPI wndproc::callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) noexcept
{
    FD_ASSERT(window == self->hwnd_);

#define ARGS window, message, w_param, l_param

    // #ifdef IMGUI_DISABLE_DEMO_WINDOWS
    //     if (!gui::menu->visible())
    //         return call_original(ARGS);
    // #endif

    std::pair args(std::pair(window, message), std::pair(w_param, l_param));

    switch (gui::Context->process_keys(&args))
    {
    case TRUE:
        return TRUE;
    case FALSE:
        return call_original(ARGS);
    default:
        return self->def_(ARGS);
    }
}