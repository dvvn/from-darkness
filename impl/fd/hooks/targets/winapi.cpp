
#include <fd/hooks/impl.h>

#include <windows.h>

import fd.gui.basic_input_handler;
import fd.application_data;
import fd.functional.invoke;

//#define HOT_UNLOAD_SUPPORTED

using namespace fd;

#define ARGS hwnd, msg, wparam, lparam

FD_HOOK(wndproc, app_info->root_window.proc.curr(), static, LRESULT WINAPI, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const auto input_result = invoke(gui::input_handler, ARGS);
    const auto block_input  = input_result.touched();
    LRESULT ret;
    if (!block_input)
        ret = call_original(ARGS);
    else if (input_result.have_return_value())
        ret = input_result.return_value();
    else
        ret = invoke(app_info->root_window.proc.def(), ARGS);
    return ret;
}
