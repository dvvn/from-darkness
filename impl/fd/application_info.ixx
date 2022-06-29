module;

#include "fd/object.h"

#include <windows.h>

export module fd.application_info;

struct rect_ex : RECT
{
    LONG width() const;
    LONG height() const;
};

struct window_hwnd
{
    HWND window_handle;
};

struct window_size : window_hwnd
{
    rect_ex full() const;
    rect_ex client() const;
};

struct wndproc_ctrl : window_hwnd
{
    WNDPROC def() const;
    WNDPROC curr() const;
    WNDPROC set(const WNDPROC proc);
};

union window_info
{
    HWND handle;
    window_size size;
    wndproc_ctrl proc;
};

window_info* get_window_info(const HWND window_handle);

struct application_info
{
    window_info window;
    HMODULE module_handle;

    application_info(const HWND window_handle, const HMODULE module_handle = nullptr);
};

export namespace fd
{
    FD_OBJECT(app_info, application_info);
    using ::get_window_info;
} // namespace fd
