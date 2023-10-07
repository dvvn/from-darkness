﻿#include "noncopyable.h"
#include "win32.h"
#include "diagnostics/system_error.h"

#include <Windows.h>
#include <tchar.h>

namespace fd
{
union packed_backend
{
    using pointer = own_win32_backend*;

  private:
    LONG_PTR user_data_;
    pointer backend_;

  public:
    packed_backend(HWND window)
        : user_data_(GetWindowLongPtr(window, GWLP_USERDATA))
    {
    }

    explicit operator bool() const
    {
        return backend_ != nullptr;
    }

    pointer operator->() const
    {
        return backend_;
    }
};

DECLSPEC_NOINLINE static LRESULT WINAPI wnd_proc(HWND window, UINT const message, WPARAM wparam, LPARAM lparam) noexcept
{
    packed_backend const backend(window);

    if (!backend)
        return DefWindowProc(window, message, wparam, lparam);

    if (message == WM_DESTROY)
    {
        // PostQuitMessage(EXIT_SUCCESS);
        backend->close();
        return NULL;
    }
    using enum win32_backend_update_response;

    return backend->update(window, message, wparam, lparam)(
        make_win32_backend_update_response<skipped>(DefWindowProc),
        make_win32_backend_update_response<updated>(DefWindowProc),
        make_win32_backend_update_response<locked>(win32_backend_update_unchanged()));
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
}

own_win32_backend_data::~own_win32_backend_data()
{
    UnregisterClass(info_.lpszClassName, info_.hInstance);
    DestroyWindow(hwnd_);
}

own_win32_backend_data::own_win32_backend_data()
{
    constexpr auto window_name = _T("") __TIMESTAMP__;
    constexpr auto class_name  = _T("") __TIME__;

#ifndef _DEBUG
    memset(&info_, 0, sizeof(WNDCLASSEX));
#endif
    info_ = {
        .cbSize        = sizeof(WNDCLASSEX), //
        .style         = CS_CLASSDC,
        .lpfnWndProc   = wnd_proc,
        .hInstance     = GetModuleHandle(nullptr),
        .lpszClassName = class_name};

    auto const class_atom = RegisterClassEx(&info_);
    if (class_atom == INVALID_ATOM)
        throw system_error("Unable to register window class!");

    win32_window_size size;
    auto const parent = GetDesktopWindow();
    if (RECT parent_rect; parent && GetWindowRect(parent, &parent_rect))
    {
        simple_win32_window_size const parent_size(parent_rect);
        size.x = parent_rect.bottom * 0.05;
        size.y = parent_rect.right * 0.05;
        size.w = parent_size.w * 0.8;
        size.h = parent_size.h * 0.8;
    }

    hwnd_ = CreateWindow(
        MAKEINTATOM(class_atom), window_name, WS_OVERLAPPEDWINDOW, //
        size.x, size.y, size.w, size.h,                            //
        parent, nullptr, info_.hInstance, nullptr);
    if (!hwnd_)
        throw system_error("Window not created!");
    /*RECT rect;
    if (!GetWindowRect(hwnd_, &rect))
        throw system_error("Unable to get window rect");*/

    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>((this)));
    ShowWindow(hwnd_, SW_SHOWDEFAULT);
    UpdateWindow(hwnd_);
}

own_win32_backend::own_win32_backend()
    : basic_win32_backend(hwnd_)
{
}

bool own_win32_backend::update()
{
    MSG msg;
    while (PeekMessage(&msg, hwnd_, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<bool>(packed_backend(hwnd_));
}

void own_win32_backend::close()
{
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, NULL);
}

void own_win32_backend::fill(win32_backend_info* info) const
{
    // info->proc = wnd_proc;
    info->id = hwnd_;
}
} // namespace fd