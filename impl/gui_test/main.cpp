#include "backend.h"

#include <fd/algorithm.h>
#include <fd/assert_impl.h>
#include <fd/exception.h>
#include <fd/format.h>
#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_callback.h>
#include <fd/hook_storage.h>
#include <fd/logger_impl.h>
#include <fd/system.h>
#include <fd/system_console.h>

#include <imgui.h>

using namespace fd;

static void _log_cpu_info(console_writer_front writer)
{
    auto support_message = [&writer, buff = string()](string_view isa_feature, bool is_supported) mutable {
        buff.clear();
        (write_string(buff, "cpu feature '", isa_feature, "' ", is_supported ? "supported" : "NOT supported"));
        writer(buff);
    };

    support_message("3DNOW", system::CPU._3DNOW);
    support_message("3DNOWEXT", system::CPU._3DNOWEXT);
    support_message("ABM", system::CPU.ABM);
    support_message("ADX", system::CPU.ADX);
    support_message("AES", system::CPU.AES);
    support_message("AVX", system::CPU.AVX);
    support_message("AVX2", system::CPU.AVX2);
    support_message("AVX512CD", system::CPU.AVX512CD);
    support_message("AVX512ER", system::CPU.AVX512ER);
    support_message("AVX512F", system::CPU.AVX512F);
    support_message("AVX512PF", system::CPU.AVX512PF);
    support_message("BMI1", system::CPU.BMI1);
    support_message("BMI2", system::CPU.BMI2);
    support_message("CLFSH", system::CPU.CLFSH);
    support_message("CMPXCHG16B", system::CPU.CMPXCHG16B);
    support_message("CX8", system::CPU.CX8);
    support_message("ERMS", system::CPU.ERMS);
    support_message("F16C", system::CPU.F16C);
    support_message("FMA", system::CPU.FMA);
    support_message("FSGSBASE", system::CPU.FSGSBASE);
    support_message("FXSR", system::CPU.FXSR);
    support_message("HLE", system::CPU.HLE);
    support_message("INVPCID", system::CPU.INVPCID);
    support_message("LAHF", system::CPU.LAHF);
    support_message("LZCNT", system::CPU.LZCNT);
    support_message("MMX", system::CPU.MMX);
    support_message("MMXEXT", system::CPU.MMXEXT);
    support_message("MONITOR", system::CPU.MONITOR);
    support_message("MOVBE", system::CPU.MOVBE);
    support_message("MSR", system::CPU.MSR);
    support_message("OSXSAVE", system::CPU.OSXSAVE);
    support_message("PCLMULQDQ", system::CPU.PCLMULQDQ);
    support_message("POPCNT", system::CPU.POPCNT);
    support_message("PREFETCHWT1", system::CPU.PREFETCHWT1);
    support_message("RDRAND", system::CPU.RDRAND);
    support_message("RDSEED", system::CPU.RDSEED);
    support_message("RDTSCP", system::CPU.RDTSCP);
    support_message("RTM", system::CPU.RTM);
    support_message("SEP", system::CPU.SEP);
    support_message("SHA", system::CPU.SHA);
    support_message("SSE", system::CPU.SSE);
    support_message("SSE2", system::CPU.SSE2);
    support_message("SSE3", system::CPU.SSE3);
    support_message("SSE4.1", system::CPU.SSE41);
    support_message("SSE4.2", system::CPU.SSE42);
    support_message("SSE4a", system::CPU.SSE4a);
    support_message("SSSE3", system::CPU.SSSE3);
    support_message("SYSCALL", system::CPU.SYSCALL);
    support_message("TBM", system::CPU.TBM);
    support_message("XOP", system::CPU.XOP);
    support_message("XSAVE", system::CPU.XSAVE);
}

int main(int, char**)
{
    backend_data backend;
    if (!backend.d3d)
        return EXIT_FAILURE;

    //---

    system_console sysConsole;

    const default_logs_handler logsCallback([&](auto msg) {
        sysConsole.out()(msg);
    });

#if defined(_DEBUG) || 1
    const default_assert_handler assertHandler([&](const assert_data& adata) {
        sysConsole.out()(parse(adata));
    });
    //_log_cpu_info(sysConsole.out());
#endif

    //----

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