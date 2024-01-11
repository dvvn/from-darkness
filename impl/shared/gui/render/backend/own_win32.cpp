#include "diagnostics/fatal.h"
#include "gui/render/backend/own_win32.h"

#include <Windows.h>
#include <tchar.h>

namespace fd::gui
{
namespace detail
{
static const WNDPROC own_wnd_proc = [](HWND window, UINT const message, WPARAM wparam, LPARAM lparam) -> LRESULT {
    union
    {
        basic_own_win32_backend* backend;
        LONG_PTR window_user_data;
    };

    window_user_data = GetWindowLongPtr(window, GWLP_USERDATA);

    if (!window_user_data)
        return DefWindowProc(window, message, wparam, lparam);

    if (message == WM_DESTROY)
    {
        // PostQuitMessage(EXIT_SUCCESS);
        backend->close();
        return NULL;
    }

    auto const result = static_cast<basic_win32_backend*>(backend)->update(window, message, wparam, lparam);
    return result.retval_or_default(window, message, wparam, lparam);

#if 0
        switch (msg) // NOLINT(hicpp-multiway-paths-covered)
        {
            /*case WM_CREATE: {
                auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lparam);
                auto d3d  = reinterpret_cast<LONG>(lpcs->lpCreateParams);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, d3d);
                break;
            }*/
        case WM_SIZE: {
            if (wparam == SIZE_MINIMIZED)
                break;

            auto &d3d = *reinterpret_cast<basic_win32_backend *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!d3d)
                break;

#ifdef RESET_BACK_BUFFER_ON_RESIZE
            if (!d3d.resize(LOWORD(lparam), HIWORD(lparam)))
                break;
            d3d.reset();
#else
#error "not implemented"
#endif
            return FALSE;
        }
        case WM_SYSCOMMAND: {
            if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return FALSE;
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return FALSE;
        }
        }
        return ::DefWindowProc(hwnd, msg, wparam, lparam);
#endif
};

own_win32_backend_data::~own_win32_backend_data()
{
    DestroyWindow(window_);
    UnregisterClass(info_.lpszClassName, info_.hInstance);
}

own_win32_backend_data::own_win32_backend_data()
{
    auto const window_name = _T("") __TIMESTAMP__;
    auto const class_name  = _T("") __TIME__;

#ifndef _DEBUG
    memset(&info_, 0, sizeof(WNDCLASSEX));
#endif
    info_ = {
        .cbSize        = sizeof(WNDCLASSEX), //
        .style         = CS_CLASSDC,
        .lpfnWndProc   = own_wnd_proc,
        .hInstance     = GetModuleHandle(nullptr),
        .lpszClassName = class_name};

    auto const class_atom = RegisterClassEx(&info_);
    assert(class_atom != INVALID_ATOM);

    int size_x, size_y, size_w, size_h;
    auto const parent = GetDesktopWindow();
    if (win::rect parent_rect; /*parent &&*/ GetWindowRect(parent, &parent_rect))
    {
        size_x = parent_rect.x * 0.05;
        size_y = parent_rect.y * 0.05;
        size_w = parent_rect.w * 0.8;
        size_h = parent_rect.h * 0.8;
    }
    else
    {
        size_x = size_y = size_w = size_h = CW_USEDEFAULT;
    }

    window_ = CreateWindow(
        MAKEINTATOM(class_atom), window_name, WS_OVERLAPPEDWINDOW, //
        size_x, size_y, size_w, size_h,                            //
        parent, nullptr, info_.hInstance, nullptr);
    assert(window_ != nullptr);

    /*RECT rect;
    if (!GetWindowRect(window_, &rect))
        throw system_error("Unable to get window rect");*/

    SetWindowLongPtr(window_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(window_, SW_SHOWDEFAULT);
    UpdateWindow(window_);
}
} // namespace detail

basic_own_win32_backend::basic_own_win32_backend()
    : basic_win32_backend(window_)
    , active_(true)
{
}

bool basic_own_win32_backend::update()
{
    MSG msg;
    while (PeekMessage(&msg, window_, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return active_;
}

void basic_own_win32_backend::close()
{
    // assert(active_ == true);
    active_ = false;
    // SetWindowLongPtr(window_, GWLP_USERDATA, NULL);
}

win::window_info basic_own_win32_backend::window() const
{
    return window_;
}
} // namespace fd::gui