
#include <fd/hooks/impl.h>

#include <windows.h>

import fd.gui.basic_input_handler;
import fd.rt_modules;

// #define HOT_UNLOAD_SUPPORTED

using namespace fd;

static BOOL CALLBACK _Wnd_Callback(HWND hwnd, LPARAM lparam)
{
    if (!IsWindowVisible(hwnd))
        return TRUE;

    TCHAR name[MAX_PATH];
    GetClassName(hwnd, name, MAX_PATH);
    WNDCLASSEX wc;
    const auto handle = GetModuleHandle(nullptr);
    const auto found  = GetClassInfoEx(GetModuleHandle(nullptr), name, &wc);
    if (!found)
        return TRUE;

    const auto wndproc = reinterpret_cast<uint8_t*>(wc.lpfnWndProc);
    const auto rng     = dos_nt(*rt_modules::current).read();
    const auto min     = &rng.front();
    const auto max     = &rng.back();
    if (wndproc < min || wndproc > max)
        return TRUE;

    *reinterpret_cast<void**>(lparam) = wc.lpfnWndProc;
    return FALSE;
}

static auto _Find_window()
{
    LONG_PTR wp;

    if (rt_modules::current->is_root())
        EnumWindows(_Wnd_Callback, reinterpret_cast<LPARAM>(&wp));
    else
        wp = GetWindowLongPtrA(FindWindowA("Valve001", nullptr), GWLP_WNDPROC);

    return reinterpret_cast<WNDPROC>(wp);
}

#define ARGS hwnd, msg, wparam, lparam

FD_HOOK(wndproc, _Find_window(), static, LRESULT WINAPI, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const auto input_result = invoke(gui::input_handler, ARGS);
    const auto block_input  = input_result.touched();
    LRESULT ret;
    if (!block_input)
        ret = call_original(ARGS);
    else if (input_result.have_return_value())
        ret = input_result.return_value();
    else
        ret = invoke(DefWindowProcW, ARGS);
    return ret;
}
