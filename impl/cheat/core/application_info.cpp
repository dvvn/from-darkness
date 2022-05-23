module;

#include <windows.h>

#include <functional>

module cheat.application_info;

LONG rect_ex::width() const noexcept
{
    return right - left;
}

LONG rect_ex::height() const noexcept
{
    return bottom - top;
}

//-----------

rect_ex window_size::full() const noexcept
{
    rect_ex out;
    GetWindowRect(handle, &out);
    return out;
}

rect_ex window_size::client() const noexcept
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

WNDPROC wndproc_ctrl::def() const noexcept
{
    return _UNI(DefWindowProc);
}

WNDPROC wndproc_ctrl::curr() const noexcept
{

    return reinterpret_cast<WNDPROC>(_UNI_FN(GetWindowLongPtr, handle, GWLP_WNDPROC));
}

WNDPROC wndproc_ctrl::set(const WNDPROC proc) noexcept
{

    return reinterpret_cast<WNDPROC>(_UNI_FN(SetWindowLong, handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc)));
}

//----------

application_info::application_info(const HWND h)
    : handle(h)
{
}

bool application_info::unicode() const noexcept
{
    return IsWindowUnicode(handle);
}
