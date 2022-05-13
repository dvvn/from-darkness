module;

#include <cheat/hooks/hook.h>

#include <windows.h>

module cheat.hooks.winapi.wndproc;
import cheat.gui.context;
import cheat.gui.input_handler;
import cheat.tools.window_info;

//#define HOT_UNLOAD_SUPPORTED

using namespace cheat;
using namespace hooks;
using namespace winapi;

CHEAT_HOOK(wndproc, static)
{
    using wndproc_t = LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM);

    wndproc_impl( )
    {
        const auto [def, curr] = tools::window_proc( );
        def_ = (wndproc_t)def;
        this->init((wndproc_t)curr, &callback);
    }

    static LRESULT WINAPI callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
    {
#define ARGS hwnd, msg, wparam, lparam
        const auto input_result = std::invoke(*instance_of<gui::input_handler>, ARGS);
        const auto block_input = input_result.touched( );
        return block_input ? get( ).def_(ARGS) : call_original(ARGS);
    }

private:
    wndproc_t def_;
};

CHEAT_HOOK_IMPL(wndproc);
