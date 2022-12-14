#include "backend.h"

#include <fd/assert_impl.h>
#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_holder.h>
#include <fd/hooked/hk_directx.h>
#include <fd/hooked/hk_winapi.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>

#include <imgui.h>

using namespace fd;

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    default_assert_handler assertCallback;
    AssertHandler = &assertCallback;

    system_console sysConsole;

    default_logs_handler logsCallback;
    Logger = &logsCallback;

    logsCallback.add([&](auto msg) {
        sysConsole.write(msg);
    });

    assertCallback.add([&](auto& adata) {
        sysConsole.write(parse_assert_data(adata));
    });

    std::pair<void*, HWND> guiCtxInit(backend.d3d, backend.hwnd);
    gui::context_impl guiCtx(&guiCtxInit, false);
    gui::Context = &guiCtx;

    gui::menu_impl menuCtx;
    gui::Menu = &menuCtx;

    gui::tab_bar testTabBar("test");
    gui::tab testTab("test2");
    testTab.store(bind_front(ImGui::Text, "Hello"));

    testTabBar.store(testTab);
    menuCtx.store(testTabBar);
    guiCtx.store([&] {
        menuCtx.render();
    });

    hook_callback<LRESULT, call_cvs::stdcall_, void, HWND, UINT, WPARAM, LPARAM> hookedWndProc(backend.info.lpfnWndProc, "WinAPI.WndProc");
    hookedWndProc.add([&](auto&, auto& ret, bool, auto... args) -> void {
        const auto val = guiCtx.process_keys(args...);
        if (val == TRUE)
            ret.emplace(TRUE);
        else if (val != FALSE)
            ret.emplace(DefWindowProc(args...));
    });

    hook_callback<void, call_cvs::stdcall_, IDirect3DDevice9, D3DPRESENT_PARAMETERS*> hookedDirectx9Reset({ backend.d3d, 16 }, "IDirect3DDevice9::Reset");
    hookedDirectx9Reset.add([&](auto&&...) {
        guiCtx.release_textures();
    });

    hook_callback<HRESULT, call_cvs::stdcall_, IDirect3DDevice9, CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*> hookedDirectx9Present({ backend.d3d, 17 },
                                                                                                                                       "IDirect3DDevice9::Present");
    hookedDirectx9Present.add([&](auto&, auto&, bool, auto thisPtr, auto...) {
        guiCtx.render(thisPtr);
    });

    return !hookedWndProc.enable() || !hookedDirectx9Reset.enable() || !hookedDirectx9Present.enable() ? EXIT_FAILURE : (backend.run(), EXIT_SUCCESS);
}