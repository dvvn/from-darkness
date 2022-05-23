module;

#include "cheat/core/object.h"

#include <windows.h>

#include <functional>

export module cheat.application_info;

#define constn const noexcept

struct rect_ex : RECT
{
    LONG width() constn;
    LONG height() constn;
};

struct window_handle
{
    HWND handle;
};

struct window_size : window_handle
{
    rect_ex full() constn;
    rect_ex client() constn;
};

struct wndproc_ctrl : window_handle
{
    WNDPROC def() constn;
    WNDPROC curr() constn;
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

    bool unicode() constn;
};

export namespace cheat
{
    CHEAT_OBJECT(app_info, application_info);
} // namespace cheat

module :private;

LONG rect_ex::width() constn
{
    return right - left;
}

LONG rect_ex::height() constn
{
    return bottom - top;
}

//-----------

rect_ex window_size::full() constn
{
    rect_ex out;
    GetWindowRect(handle, &out);
    return out;
}

rect_ex window_size::client() constn
{
    rect_ex out;
    GetClientRect(handle, &out);
    return out;
}

//-----------

#undef DefWindowProc
#undef GetWindowLongPtr
#undef SetWindowLong

#define _UNI(_FN_) IsWindowUnicode(handle) ? _FN_##W : _FN_##A
#define _UNI_FN(_FN_, ...) std::invoke(_UNI(_FN_), __VA_ARGS__)

WNDPROC wndproc_ctrl::def() constn
{
    return _UNI(DefWindowProc);
}

WNDPROC wndproc_ctrl::curr() constn
{

    return reinterpret_cast<WNDPROC>(_UNI_FN(GetWindowLongPtr, handle, GWLP_WNDPROC));
}

WNDPROC wndproc_ctrl::set(const WNDPROC proc) noexcept
{

    return reinterpret_cast<WNDPROC>(_UNI_FN(SetWindowLong, handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc)));
}

//----------

application_info::application_info(const HWND h) : handle(h)
{
}

bool application_info::unicode() constn
{
    return IsWindowUnicode(handle);
}

//-----------
