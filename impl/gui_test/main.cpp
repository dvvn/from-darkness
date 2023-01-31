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

int main(int, char**)
{
    using namespace fd;

    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    system_console sysConsole;

    const default_logs_handler logsCallback([&](auto msg) {
        sysConsole.write(msg);
    });

#ifdef _DEBUG
    const default_assert_handler assertHandler([&](const assert_data& adata) {
        sysConsole.write(parse(adata));
    });
#endif

    gui::menu_impl menu(
        gui::tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
            "tab bar1",
#endif
            gui::tab(
                "tab1",
                [] {
                    ImGui::TextUnformatted("hello");
                }
            ),
            gui::tab(
                "tab2",
                [] {
                    ImGui::TextUnformatted("-->hello again");
                }
            )
        ),
        gui::tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
            "tab bar2",
#endif
            gui::tab(
                "new tab",
                [] {
                    ImGui::TextUnformatted("im here!");
                }
            ),
            gui::tab(
                "tab 3",
                [] {
                    ImGui::TextUnformatted("yes!");
                }
            )
        )
    );

    gui::context guiCtx([&] {
        menu.render();
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        ImGui::ShowDemoWindow();
#endif
    });
    guiCtx.init(true);
    guiCtx.init(backend.hwnd);
    guiCtx.init(backend.d3d);

    hooks_storage2 allHooks(
        hook_callback(
            "WinAPI.WndProc",
            backend.info.lpfnWndProc,
            [&](auto orig, auto... args) -> LRESULT {
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
            }
        ),
        hook_callback(
            "IDirect3DDevice9::Reset",
            &IDirect3DDevice9::Reset,
            decay_fn(backend.d3d, 16),
            [&](auto orig, auto, auto... args) {
                guiCtx.release_textures();
                return orig(args...);
            }
        ),
        hook_callback(
            "IDirect3DDevice9::Present",
            &IDirect3DDevice9::Present,
            decay_fn(backend.d3d, 17),
            [&](auto orig, auto thisPtr, auto... args) {
                guiCtx.render(thisPtr);
                return orig(args...);
            }
        )
    );

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