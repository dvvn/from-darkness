
#include <cheat/hooks/impl.h>

#include <windows.h>

import cheat.gui.input_handler;
import cheat.application_info;

//#define HOT_UNLOAD_SUPPORTED

using namespace cheat;

#define ARGS hwnd, msg, wparam, lparam

CHEAT_HOOK(app_info->window.proc.curr(), wndproc, static, LRESULT WINAPI, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    const auto input_result = std::invoke(*gui::input_handler, ARGS);
    const auto block_input = input_result.touched();
    return block_input ? std::invoke(app_info->window.proc.def(), ARGS) : call_original(ARGS);
}
