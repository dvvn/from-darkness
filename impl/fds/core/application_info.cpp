module;

#include <nstd/runtime_assert.h>

#include <windows.h>

#include <functional>

module fds.application_info;

LONG rect_ex::width() const
{
    return right - left;
}

LONG rect_ex::height() const
{
    return bottom - top;
}

//-----------

rect_ex window_size::full() const
{
    rect_ex out;
    GetWindowRect(window_handle, &out);
    return out;
}

rect_ex window_size::client() const
{
    rect_ex out;
    GetClientRect(window_handle, &out);
    return out;
}

//-----------

#undef DefWindowProc
#undef GetWindowLongPtr
#undef SetWindowLong
#undef GetModuleHandle

#define _UNI(_FN_)         IsWindowUnicode(window_handle) ? _FN_##W : _FN_##A
#define _UNI_FN(_FN_, ...) std::invoke(_UNI(_FN_), __VA_ARGS__)

WNDPROC wndproc_ctrl::def() const
{
    return _UNI(DefWindowProc);
}

WNDPROC wndproc_ctrl::curr() const
{
    return reinterpret_cast<WNDPROC>(_UNI_FN(GetWindowLongPtr, window_handle, GWLP_WNDPROC));
}

WNDPROC wndproc_ctrl::set(const WNDPROC proc)
{
    return reinterpret_cast<WNDPROC>(_UNI_FN(SetWindowLong, window_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc)));
}

//----------

application_info::application_info(const HWND window_handle, const HMODULE module_handle)
    : module_handle(module_handle ? module_handle : (IsWindowUnicode(window_handle) ? GetModuleHandleW(nullptr) : GetModuleHandleA(nullptr)))
{
    window.handle = window_handle;
}
