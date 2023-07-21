#include "name.h"
#include "noncopyable.h"
#include "win32.h"
#include "diagnostics/system_error.h"

#include <Windows.h>
#include <tchar.h>

namespace fd
{
DECLSPEC_NOINLINE static LRESULT WINAPI wnd_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    union
    {
        LONG_PTR user_data;
        basic_own_win32_backend *backend;
    };

    user_data = GetWindowLongPtr(window, GWLP_USERDATA);

    if (!backend)
        return DefWindowProc(window, message, wparam, lparam);

    if (message == WM_DESTROY)
    {
        // PostQuitMessage(EXIT_SUCCESS);
        backend->close();
        return NULL;
    }

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

class own_window_info : public noncopyable
{
    WNDCLASSEX info_;
    HWND hwnd_;

  public:
    ~own_window_info()
    {
        UnregisterClass(info_.lpszClassName, info_.hInstance);
        DestroyWindow(hwnd_);
    }

    own_window_info(LPCTSTR name, HMODULE handle, HWND parent)
    {
        // memset(&info_, 0, sizeof info_);
        info_ = {
            .cbSize        = sizeof info_,
            .style         = CS_CLASSDC,
            .lpfnWndProc   = wnd_proc,
            .hInstance     = handle,
            .lpszClassName = own_backend_class_name};
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

    WNDPROC wndproc() const
    {
        return info_.lpfnWndProc;
    }

    HWND handle() const
    {
        return hwnd_;
    }
};

struct own_window_info_proxy
{
    own_window_info info;

    own_window_info_proxy(LPCTSTR name, HMODULE handle, HWND parent)
        : info(name, handle, parent)
    {
    }
};

class own_win32_backend final : own_window_info_proxy, public basic_own_win32_backend
{
    bool closed_;
    bool minimized_;
    window_size size_;

    update_result update(HWND window, UINT message, WPARAM wparam, LPARAM lparam) override
    {
        if (message == WM_SIZE)
        {
            minimized_ = wparam == SIZE_MINIMIZED;
            if (!minimized_)
                size_ = lparam;
        }

        return basic_win32_backend::update(window, message, wparam, lparam);

        // return DefWindowProc(window, message, wparam, lparam);
    }

  public:
    ~own_win32_backend() override
    {
        basic_win32_backend::destroy();
    }

    own_win32_backend(HWND parent = GetDesktopWindow())
        : own_window_info_proxy(_T("") __TIMESTAMP__, GetModuleHandle(nullptr), parent)
        , basic_own_win32_backend(info.handle())
    {
        auto window = info.handle();

        RECT rect;
        if (!GetWindowRect(window, &rect))
            throw system_error("Unable to get window rect");

        closed_    = false;
        minimized_ = false;
        size_      = rect;

        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(static_cast<basic_win32_backend *>(this)));
        ShowWindow(window, SW_SHOWDEFAULT);
        UpdateWindow(window);
    }

    void peek() override
    {
        MSG msg;
        while (PeekMessage(&msg, info.handle(), 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    WNDPROC proc() const override
    {
        return info.wndproc();
    }

    HWND id() const override
    {
        return info.handle();
    }

    bool minimized() const override
    {
        return minimized_; // IsIconic
    }

    window_size size() const override
    {
        return size_; // GetWindowRect
    }

    void close() override
    {
        assert(!closed_);
        closed_ = true;
    }

    bool closed() const override
    {
        return closed_;
    }
};

FD_OBJECT_IMPL(own_win32_backend);
} // namespace fd