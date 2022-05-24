module;

#include "cheat/core/object.h"

#include <windows.h>

export module cheat.application_info;

struct rect_ex : RECT
{
    LONG width() const noexcept;
    LONG height() const noexcept;
};

struct window_hwnd
{
    HWND window_handle;
};

struct window_size : window_hwnd
{
    rect_ex full() const noexcept;
    rect_ex client() const noexcept;
};

struct wndproc_ctrl : window_hwnd
{
    WNDPROC def() const noexcept;
    WNDPROC curr() const noexcept;
    WNDPROC set(const WNDPROC proc) noexcept;
};

//----

struct application_info
{
    union
    {
        HWND handle;
        window_size size;
        wndproc_ctrl proc;
    } window;

    HMODULE module_handle;

    application_info(const HWND window_handle, const HMODULE module_handle);

#ifdef _DEBUG
    application_info() = default;
#endif
};

constexpr size_t _App_info_idx = 0;

export namespace cheat
{
    CHEAT_OBJECT(app_info, application_info, _App_info_idx);
}
