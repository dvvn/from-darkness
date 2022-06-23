
#include <fd/hooks/impl.h>

#include <windows.h>

import fd.gui.input_handler;
import fd.application_info;

//#define HOT_UNLOAD_SUPPORTED

using namespace fd;

#define ARGS hwnd, msg, wparam, lparam

FD_HOOK(app_info->window.proc.curr(), static, LRESULT WINAPI, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const auto input_result = std::invoke(gui::input_handler, ARGS);
    const auto block_input  = input_result.touched();
    return block_input ? std::invoke(app_info->window.proc.def(), ARGS) : call_original(ARGS);
}
