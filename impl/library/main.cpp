#ifdef _DEBUG
#include <fd/assert_impl.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>
#endif

#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_holder.h>
#include <fd/hooked/hk_directx.h>
#include <fd/hooked/hk_vgui_surface.h>
#include <fd/hooked/hk_winapi.h>
#include <fd/library_info.h>

#include <d3d9.h>
#include <windows.h>

static HMODULE _ModuleHandle;

static HANDLE _ThreadHandle;
static DWORD _ThreadId = 0;

using namespace fd;

static void _fail()
{
    // TerminateThread(_ThreadHandle, EXIT_FAILURE);
    // FreeLibrary(ModuleHandle);
    FreeLibraryAndExitThread(_ModuleHandle, EXIT_FAILURE);
};

static DWORD WINAPI _loader(void*) noexcept
{
    const lazy_invoke resetThreadId([] {
        _ThreadId = 0;
    });

    [[maybe_unused]] const auto stdTerminate = std::set_terminate(_fail);

    CurrentLibraryHandle = _ModuleHandle;

#ifdef _DEBUG
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
#endif

    const library_info clientLib(L"client.dll", true);
    const auto addToSafeList = static_cast<void(__fastcall*)(HMODULE, void*)>(clientLib.find_signature("56 8B 71 3C B8"));

    invoke(addToSafeList, _ModuleHandle, nullptr);

    const auto d3dIfc = [] {
        const library_info lib(L"shaderapidx9.dll", true);
        const auto addr = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3dParams;
        if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
            _fail();
        return d3dParams.hFocusWindow;
    }();

    std::pair guiData(d3dIfc, hwnd);
    gui::context_impl guiCtx(&guiData, false);
    gui::Context = &guiCtx;

    gui::menu_impl menuCtx;
    gui::Menu = &menuCtx;

    hook_holder allHooks(hooked::d3d9_reset({ d3dIfc, 16 }),
                         hooked::d3d9_present({ d3dIfc, 17 }),
                         hooked::wndproc(hwnd, GetWindowLongPtrW(hwnd, GWLP_WNDPROC)),
                         hooked::lock_cursor({ (void*)0, 67 }));

    if (!allHooks.enable())
        _fail();

    std::set_terminate([] {
        if (ResumeThread(_ThreadHandle) == -1)
            std::abort();
    });

    /*if (!library_info::_Wait(L"serverbrowser.dll"))
        _Fail();*/

    if (SuspendThread(_ThreadHandle) == -1)
        _fail();

    if (!allHooks.disable())
        std::abort();

    return EXIT_SUCCESS;
}

// ReSharper disable once CppInconsistentNaming
extern "C" BOOL APIENTRY DllMain(const HMODULE moduleHandle, const DWORD reason, LPVOID reserved)
{
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        _ModuleHandle = moduleHandle;
        _ThreadHandle = CreateThread(nullptr, 0, _loader, nullptr, 0, &_ThreadId);
        if (!_ThreadId)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        if (_ThreadId && !CloseHandle(_ThreadHandle))
            return FALSE;
        break;
    }
    }

    return TRUE;
}