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

    gui::context_impl guiCtx(&backend, false);
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

    hook_holder allHooks(hooked::d3d9_reset({ backend.d3d, 16 }), hooked::d3d9_present({ backend.d3d, 17 }), hooked::wndproc(backend.hwnd, backend.info.lpfnWndProc));

    return allHooks.enable() ? (backend.run(), EXIT_SUCCESS) : EXIT_FAILURE;
}
