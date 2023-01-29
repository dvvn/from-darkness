#ifdef _DEBUG
#include <fd/assert_impl.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>
#endif

#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_callback.h>
#include <fd/hook_storage.h>
#include <fd/library_info.h>

#include <fd/valve/gui/surface.h>

#include <d3d9.h>
#include <windows.h>

static HMODULE _ModuleHandle;

static HANDLE _ThreadHandle;
static DWORD  _ThreadId = 0;

[[noreturn]] static void _exit_fail()
{
    // TerminateThread(_ThreadHandle, EXIT_FAILURE);
    // FreeLibrary(ModuleHandle);
    FreeLibraryAndExitThread(_ModuleHandle, EXIT_FAILURE);
}

[[noreturn]] static void _exit_success()
{
    FreeLibraryAndExitThread(_ModuleHandle, EXIT_SUCCESS);
}

static DWORD WINAPI _loader(void*) noexcept
{
    using namespace fd;

    set_unload(_exit_fail);
    set_current_library(_ModuleHandle);

    system_console sysConsole;

    const default_logs_handler logsCallback([&](auto msg) {
        sysConsole.write(msg);
    });

#ifdef _DEBUG
    const default_assert_handler assertHandler([&](const assert_data& adata) {
        sysConsole.write(parse(adata));
    });
#endif

    const auto clientLib     = wait_for_library(L"client.dll");
    const auto addToSafeList = reinterpret_cast<void(__fastcall*)(HMODULE, void*)>(clientLib.find_signature("56 8B 71 3C B8"));

    addToSafeList(_ModuleHandle, nullptr);

    const auto d3dIfc = [] {
        const auto lib  = wait_for_library(L"shaderapidx9.dll");
        const auto addr = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3dParams;
        if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
            _exit_fail();
        return d3dParams.hFocusWindow;
    }();

    gui::menu_impl menu(gui::tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
        "tab bar",
#endif
        gui::tab(
            "tab",
            [] {
                ImGui::TextUnformatted("test");
            }
        )
    ));
    gui::context guiCtx([&] {
        [[maybe_unused]] const auto visible = menu.render();
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
        if (visible)
            ImGui::ShowDemoWindow();
#endif
    });
    guiCtx.init(false);
    guiCtx.init(hwnd);
    guiCtx.init(d3dIfc);

    hooks_storage2 allHooks(
        hook_callback(
            "WinAPI.WndProc",
            DefWindowProcW,
            decay_fn(GetWindowLongPtrW(hwnd, GWLP_WNDPROC)),
            [&](auto orig, HWND currHwnd, auto... args) -> LRESULT {
#ifdef _DEBUG
                if (currHwnd != hwnd)
                    assertHandler.run_panic({ "Unknown HWND detected!" });
#endif
                switch (guiCtx.process_keys(currHwnd, args...))
                {
                case gui::process_keys_result::instant:
                    return TRUE;
                case gui::process_keys_result::native:
                    return orig(currHwnd, args...);
                case gui::process_keys_result::def:
                    return DefWindowProcW(currHwnd, args...);
                default:
                    unreachable();
                }
            }
        ),
        hook_callback(
            "IDirect3DDevice9::Reset",
            &IDirect3DDevice9::Reset,
            decay_fn(d3dIfc, 16),
            [&](auto orig, auto, auto... args) {
                guiCtx.release_textures();
                return orig(args...);
            }
        ),
        hook_callback(
            "IDirect3DDevice9::Present",
            &IDirect3DDevice9::Present,
            decay_fn(d3dIfc, 17),
            [&](auto orig, auto thisPtr, auto... args) {
                guiCtx.render(thisPtr);
                return orig(args...);
            }
        ),
        hook_callback(
            "VGUI.ISurface::LockCursor",
            &valve::gui::surface::LockCursor,
            decay_fn((void*)nullptr, 67),
            [&](auto orig, auto thisPtr) {
                if (menu.visible() && !thisPtr->IsCursorVisible())
                {
                    thisPtr->UnlockCursor();
                    return;
                }
                orig();
            }
        )
    );

    if (!allHooks.enable())
        _exit_fail();

    set_unload([] {
        if (ResumeThread(_ThreadHandle) == -1)
            suspend();
    });

#if 0
    if (!wait_for_library(L"serverbrowser.dll"))
        _exit_fail();
#endif

    if (SuspendThread(_ThreadHandle) == -1)
        _exit_fail();

    if (!allHooks.disable())
        _exit_fail();

    _exit_success();
}

// ReSharper disable once CppInconsistentNaming
BOOL APIENTRY DllMain(const HMODULE moduleHandle, const DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        if (!DisableThreadLibraryCalls(moduleHandle))
            return FALSE;
        _ModuleHandle = moduleHandle;
        _ThreadHandle = CreateThread(nullptr, 0, _loader, nullptr, 0, &_ThreadId);
        if (!_ThreadId)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        if (_ThreadId && WaitForSingleObject(_ThreadHandle, INFINITE) == WAIT_FAILED)
            return FALSE;
        break;
    }
    }

    return TRUE;
}