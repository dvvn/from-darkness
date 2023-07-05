#include "win32.h"
//
#include "diagnostics/system_error.h"

#include <tchar.h>

namespace fd
{
DECLSPEC_NOINLINE static LRESULT WINAPI wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_DESTROY)
    {
        PostQuitMessage(EXIT_SUCCESS);
        return NULL;
    }

    union
    {
        LONG_PTR user_data;
        basic_win32_backend *backend;
    };

    user_data = GetWindowLongPtr(window, GWLP_USERDATA);

    if (!backend)
        return DefWindowProc(window, message, wparam, lparam);

    return backend->update(window, message, wparam, lparam).finish(DefWindowProc, window, message, wparam, lparam);
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

own_window_info::~own_window_info()
{
    UnregisterClass(info_.lpszClassName, info_.hInstance);
    DestroyWindow(hwnd_);
}

own_window_info::own_window_info(LPCTSTR name, HMODULE handle, HWND parent)
{
    // memset(&info_, 0, sizeof info_);
    info_ = {
        .cbSize        = sizeof info_,
        .style         = CS_CLASSDC,
        .lpfnWndProc   = wnd_proc,
        .hInstance     = handle,
        .lpszClassName = _T("__backend_win32")};
    auto class_atom = RegisterClassEx(&info_);
    if (class_atom == INVALID_ATOM)
        throw system_error("Unable to register class!");

    RECT parent_rect;
    int x, y, width, height; // NOLINT(readability-isolate-declaration)
    if (parent && GetWindowRect(parent, &parent_rect))
    {
        // ReSharper disable CppClangTidyClangDiagnosticFloatConversion
        // ReSharper disable CppClangTidyBugproneNarrowingConversions
        // ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#pragma warning(push)
#pragma warning(disable : 4244)
        float w = parent_rect.right - parent_rect.left;
        float h = parent_rect.bottom - parent_rect.top;
        x       = w * 0.1f / 2.f;
        y       = h * 0.1f / 2.f;
        width   = w * 0.9f;
        height  = h * 0.9f;
#pragma warning(pop)
    }
    else
    {
        x = y = width = height = CW_USEDEFAULT;
    }

    hwnd_ = CreateWindow(
        MAKEINTATOM(class_atom), name, WS_OVERLAPPEDWINDOW, x, y, width, height, parent, nullptr, handle, nullptr);

    if (!hwnd_)
        throw system_error("Window not created!");
}

WNDPROC own_window_info::wndproc() const
{
    return info_.lpfnWndProc;
}

HWND own_window_info::handle() const
{
    return hwnd_;
}

//-------

auto win32_backend_own::update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) -> update_result
{
    if (message == WM_SIZE)
    {
        params_.minimized = wparam == SIZE_MINIMIZED;
        if (!params_.minimized)
        {
            params_.w = LOWORD(lparam);
            params_.h = HIWORD(lparam);
        }
    }

    return basic_win32_backend::update(window, message, wparam, lparam);

    // return DefWindowProc(window, message, wparam, lparam);
}

win32_backend_own::~win32_backend_own()
{
    basic_win32_backend::destroy();
}

win32_backend_own::win32_backend_own(HWND parent)
    : own_window_info(_T("") __TIMESTAMP__, GetModuleHandle(nullptr), parent)
    , basic_win32_backend(handle())
{
    auto window = handle();

    RECT rect;
    if (!GetWindowRect(window, &rect))
        throw system_error("Unable to get window rect");
    params_.w = rect.right - rect.left;
    params_.h = rect.bottom - rect.top;

    SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<basic_win32_backend *>(this)));
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
}

auto win32_backend_own::peek() -> window_params *
{
    MSG msg;
    auto quit = false;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        quit |= msg.message == WM_QUIT;
    }

    return quit ? nullptr : &params_;
}

WNDPROC win32_backend_own::proc() const
{
    return wndproc();
}

HWND win32_backend_own::id() const
{
    return handle();
}
} // namespace fd