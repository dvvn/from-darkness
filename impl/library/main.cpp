#include <fd/hooks/helper.h>

#include <windows.h>

#include <exception>

#ifdef _DEBUG
import fd.logger.impl;
import fd.assert.impl;
import fd.system.console;
#endif

import fd.gui.context;
import fd.gui.menu.impl;

import fd.hooks.directx;
import fd.hooks.winapi;

import fd.library_info;
import fd.functional.lazy_invoke;

static HMODULE _Handle;

static HANDLE _T_Handle;
static DWORD _T_id = 0;

struct IDirect3DDevice9;

using namespace fd;

static DWORD WINAPI _Loader(void*) noexcept
{
    const lazy_invoke reset_id_helper = [] {
        _T_id = 0;
    };

    const auto std_terminate = std::set_terminate([] {
        TerminateThread(_T_Handle, EXIT_FAILURE);
        FreeLibrary(_Handle);
    });

    current_library_handle = _Handle;

#ifdef _DEBUG
    default_assert_handler assert_callback;
    assert_handler = &assert_callback;

    system_console sys_console;

    default_logs_handler logs_callback;
    logger = &logs_callback;

    logs_callback.add([&](auto msg) {
        sys_console.write(msg);
    });

    assert_callback.add([&](auto& adata) {
        sys_console.write(parse_assert_data(adata));
    });
#endif

    const library_info client_lib = { L"client.dll", true };
    const auto add_to_safe_list   = static_cast<void(__fastcall*)(HMODULE, void*)>(client_lib.find_signature("56 8B 71 3C B8"));

    add_to_safe_list(_Handle, nullptr);

    const gui::context gui_ctx;
    gui::menu_impl menu_ctx;
    gui::menu = &menu_ctx;

    const auto d3d_ifc = [] {
        const library_info lib = { L"shaderapidx9.dll", true };
        const auto addr        = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    hooks::holder all_hooks = { hooks::d3d9_reset(d3d_ifc), hooks::d3d9_present(d3d_ifc), hooks::wndproc("Valve001") };

    if (!all_hooks.enable())
        FreeLibraryAndExitThread(_Handle, EXIT_FAILURE);

    std::set_terminate([] {
        if (ResumeThread(_T_Handle) == -1)
            std::abort();
    });

    // if (!library_info::_Wait(L"serverbrowser.dll"))
    //   FreeLibraryAndExitThread(_Handle, EXIT_FAILURE);

    if (SuspendThread(_T_Handle) == -1)
        FreeLibraryAndExitThread(_Handle, EXIT_FAILURE);

    if (!all_hooks.disable())
        std::abort();

    return EXIT_SUCCESS;
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH: {
        _Handle   = hModule;
        _T_Handle = CreateThread(nullptr, 0, _Loader, nullptr, 0, &_T_id);
        if (!_T_id)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        if (_T_id && !CloseHandle(_T_Handle))
            return FALSE;
        break;
    }
    }

    return TRUE;
}
