#ifdef _DEBUG
#include <fd/assert_impl.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>
#endif

#include <fd/functional.h>
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

using namespace fd;

static void _exit_fail()
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
    const lazy_invoke onReturn([] {
        _ThreadId = 0;
    });
    set_unload(_exit_fail);

    set_current_module_handle(_ModuleHandle);

    system_console sysConsole;

    const default_logs_handler logsCallback([&](auto msg) {
        sysConsole.write(msg);
    });

#ifdef _DEBUG
    const default_assert_handler assertHandler([&](const assert_data& adata) {
        sysConsole.write(parse(adata));
    });
#endif

    const library_info clientLib(L"client.dll", true);
    const auto         addToSafeList = reinterpret_cast<void(__fastcall*)(HMODULE, void*)>(clientLib.find_signature("56 8B 71 3C B8"));

    addToSafeList(_ModuleHandle, nullptr);

    const auto d3dIfc = [] {
        const library_info lib(L"shaderapidx9.dll", true);
        const auto         addr = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3dParams;
        if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
            _exit_fail();
        return d3dParams.hFocusWindow;
    }();

    gui::context guiCtx(d3dIfc, hwnd);
    gui::menu    menu(&guiCtx);
    guiCtx.store([&] {
        menu.render();
    });
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    guiCtx.store([&] {
        if (menu.visible())
            ImGui::ShowDemoWindow();
    });
#endif

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
                switch (guiCtx.process_keys(hwnd, args...))
                {
                case gui::process_keys_result::instant:
                    return TRUE;
                case gui::process_keys_result::native:
                    return orig(hwnd, args...);
                case gui::process_keys_result::def:
                    return DefWindowProcW(hwnd, args...);
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

    /*if (!library_info::_Wait(L"serverbrowser.dll"))
        _Fail();*/

    if (SuspendThread(_ThreadHandle) == -1)
        _exit_fail();

    if (!allHooks.disable())
        _exit_fail();

    _exit_success();
}

// ReSharper disable once CppInconsistentNaming
extern "C" BOOL APIENTRY DllMain(const HMODULE moduleHandle, const DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        _ModuleHandle = moduleHandle;
        _ThreadHandle = CreateThread(nullptr, 0, _loader, nullptr, 0, &_ThreadId);
        if (_ThreadId == 0u)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        if (_ThreadId != 0u && CloseHandle(_ThreadHandle) == 0)
            return FALSE;
        break;
    }
    }

    return TRUE;
}