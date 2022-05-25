module;

#include <cheat/hooks/impl.h>

#include <windows.h>

module cheat.hooks.winapi.wndproc;
import cheat.gui.input_handler;
import cheat.application_info;

//#define HOT_UNLOAD_SUPPORTED

using namespace cheat;
using namespace hooks;
using namespace winapi;

#define ARGS_T HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam
#define ARGS hwnd, msg, wparam, lparam

CHEAT_HOOK_BODY(wndproc, static, LRESULT WINAPI, ARGS_T);

CHEAT_HOOK_INIT(wndproc)
{
    const auto proc = app_info->window.proc;
    // def_ = proc.def();
    hook::init(proc.curr(), &callback);
}

CHEAT_HOOK_CALLBACK(wndproc, LRESULT WINAPI, ARGS_T)
{
    const auto input_result = std::invoke(*gui::input_handler, ARGS);
    const auto block_input = input_result.touched();
    return block_input ? std::invoke(app_info->window.proc.def(), ARGS) : call_original(ARGS);
}
