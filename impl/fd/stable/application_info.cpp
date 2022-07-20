module;

#include <fd/assert.h>

#include <windows.h>

module fd.application_info;

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

#define _UNI(_FN_, ...)    IsWindowUnicode(window_handle) ? _FN_##W##__VA_ARGS__ : _FN_##A##__VA_ARGS__
#define _UNI_FN(_FN_, ...) _UNI(_FN_, (__VA_ARGS__))

WNDPROC wndproc_ctrl::def() const
{
    const auto ret = _UNI(DefWindowProc);
    return ret;
}

WNDPROC wndproc_ctrl::curr() const
{
    const auto ret = _UNI_FN(GetWindowLongPtr, window_handle, GWLP_WNDPROC);
    return reinterpret_cast<WNDPROC>(ret);
}

WNDPROC wndproc_ctrl::set(const WNDPROC proc)
{
    const auto ret = _UNI_FN(SetWindowLong, window_handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc));
    return reinterpret_cast<WNDPROC>(ret);
}

//----------

static auto _Correct_module_handle(const HWND hwnd)
{
    return IsWindowUnicode(hwnd) ? GetModuleHandleW(nullptr) : GetModuleHandleA(nullptr);
}

application_info::application_info(const HWND window_handle, const HMODULE module_handle)
    : module_handle(module_handle ? module_handle : _UNI_FN(GetModuleHandle, nullptr))
{
    // FD_ASSERT(window_handle != nullptr);
    // FD_ASSERT(this->module_handle != nullptr);
    this->window.handle = window_handle;
}
