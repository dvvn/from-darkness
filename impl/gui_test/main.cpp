#include "backend.h"

#include <fd/assert_impl.h>
#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_callback.h>
#include <fd/hook_storage.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>

#include <imgui.h>

using namespace fd;

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

#ifdef _DEBUG
    default_assert_handler assertCallback;
    AssertHandler = &assertCallback;
#endif

    system_console sysConsole;

    default_logs_handler logsCallback;
    Logger = &logsCallback;

    logsCallback.add([&](auto msg) {
        sysConsole.write(msg);
    });

#ifdef _DEBUG
    assertCallback.add([&](auto& adata) {
        sysConsole.write(parse_assert_data(adata));
    });
#endif

    gui::context guiCtx(backend.d3d, backend.hwnd);
    gui::menu    menu(&guiCtx);

    guiCtx.store([&] {
        menu.render();
    });

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    guiCtx.store([] {
        ImGui::ShowDemoWindow();
    });
#endif

    gui::tab_bar testTabBar("test");
    gui::tab     testTab("test2");
    testTab.store(bind_front(ImGui::Text, "Hello"));

    testTabBar.store(testTab);
    menu.store(testTabBar);

    hooks_storage allHooks;

    hook_callback hkWndProc(backend.info.lpfnWndProc, [&](auto orig, auto... args) -> LRESULT {
        switch (guiCtx.process_keys(args...))
        {
        case gui::process_keys_result::instant:
            return TRUE;
        case gui::process_keys_result::native:
            return orig(args...);
        case gui::process_keys_result::def:
            return DefWindowProc(args...);
        default:
            unreachable();
        }
    });
    hkWndProc.set_name("WinAPI.WndProc");
    allHooks.store(hkWndProc);

    hook_callback hkDirectx9Reset(&IDirect3DDevice9::Reset, decay_fn(backend.d3d, 16), [&](auto orig, auto, auto... args) {
        guiCtx.release_textures();
        return orig(args...);
    });
    hkDirectx9Reset.set_name("IDirect3DDevice9::Reset");
    allHooks.store(hkDirectx9Reset);

    hook_callback hkDirectx9Present(&IDirect3DDevice9::Present, decay_fn(backend.d3d, 17), [&](auto orig, auto thisPtr, auto... args) {
        guiCtx.render(thisPtr);
        return orig(args...);
    });
    hkDirectx9Present.set_name("IDirect3DDevice9::Present");
    allHooks.store(hkDirectx9Present);

    if (allHooks.enable())
    {
        set_unload([] {
            PostQuitMessage(EXIT_SUCCESS);
            set_unload(nullptr);
        });

        backend.run();
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}