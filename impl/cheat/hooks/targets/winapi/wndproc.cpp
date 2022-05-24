module;

#include <cheat/hooks/hook.h>

#include <windows.h>

module cheat.hooks.winapi.wndproc;
import cheat.gui.input_handler;
import cheat.application_info;

//#define HOT_UNLOAD_SUPPORTED

using namespace cheat;
using namespace hooks;
using namespace winapi;

CHEAT_HOOK(wndproc, static)
{
    wndproc_impl()
    {
        const auto proc = app_info->window.proc;
        def_ = proc.def();
        this->init(proc.curr(), &callback);
    }

    static LRESULT WINAPI callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
    {
#define ARGS hwnd, msg, wparam, lparam
        const auto input_result = std::invoke(*gui::input_handler, ARGS);
        const auto block_input = input_result.touched();
        return block_input ? get().def_(ARGS) : call_original(ARGS);
    }

  private:
    WNDPROC def_;
};

CHEAT_HOOK_IMPL(wndproc);
