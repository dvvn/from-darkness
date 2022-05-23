module;

#include "cheat/core/object.h"

#include <windows.h>

export module cheat.application_info;

struct rect_ex : RECT
{
    LONG width() const noexcept;
    LONG height() const noexcept;
};

struct window_handle
{
    HWND handle;
};

struct window_size : window_handle
{
    rect_ex full() const noexcept;
    rect_ex client() const noexcept;
};

struct wndproc_ctrl : window_handle
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
    };

    application_info(const HWND h);

    bool unicode() const noexcept;
};

export namespace cheat
{
    CHEAT_OBJECT(app_info, application_info);
}
