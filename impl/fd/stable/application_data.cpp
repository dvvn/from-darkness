module;

#include <fd/assert.h>

#include <windows.h>

module fd.application_data;

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

#ifndef UNICODE
#undef DefWindowProc
#undef GetWindowLongPtr
#undef SetWindowLong
#undef GetModuleHandle
#define _UNI(_FN_, ...) IsWindowUnicode(window_handle) ? _FN_##W##__VA_ARGS__ : _FN_##A##__VA_ARGS__
#else
#define _UNI(_FN_, ...) _FN_(__VA_ARGS__)
#endif
#define _UNI_FN(_FN_, ...) (_UNI(_FN_, (__VA_ARGS__)))

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

application_data::application_data(const HWND window_handle, const HMODULE module_handle)
    : module_handle(module_handle ? module_handle : _UNI_FN(GetModuleHandle, nullptr))
{
    // FD_ASSERT(window_handle != nullptr);
    FD_ASSERT(this->module_handle != nullptr);
    this->root_window.handle = window_handle;
}

/* [[noreturn]] static DWORD WINAPI _Unload_delayed(LPVOID)
{
    Sleep(1000);
    FreeLibraryAndExitThread(fd::app_info->module_handle, FALSE);
} */

void application_data::unload() const
{
#ifndef UNICODE
    const auto window_handle = root_window.handle;
#endif

    if (module_handle == _UNI_FN(GetModuleHandle, nullptr))
        PostQuitMessage(FALSE);
    else
        FreeLibrary(module_handle);
}
