#include <d3d9.h>
#include <windows.h>

#include <exception>

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

static HMODULE _Handle;

static HANDLE _T_Handle;
static DWORD _T_id = 0;

using namespace fd;

static DWORD WINAPI _Loader(void*) noexcept
{
    const lazy_invoke reset_id_helper = [] {
        _T_id = 0;
    };

    constexpr auto _Fail = [] {
        // TerminateThread(_T_Handle, EXIT_FAILURE);
        // FreeLibrary(_Handle);
        FreeLibraryAndExitThread(_Handle, EXIT_FAILURE);
    };

    const auto std_terminate = std::set_terminate(_Fail);

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

    const library_info client_lib(L"client.dll", true);
    const auto add_to_safe_list = static_cast<void(__fastcall*)(HMODULE, void*)>(client_lib.find_signature("56 8B 71 3C B8"));

    add_to_safe_list(_Handle, nullptr);

    const auto d3d_ifc = [] {
        const library_info lib(L"shaderapidx9.dll", true);
        const auto addr = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3d_params;
        if (FAILED(d3d_ifc->GetCreationParameters(&d3d_params)))
            _Fail();
        return d3d_params.hFocusWindow;
    }();

    std::pair gui_data(d3d_ifc, hwnd);
    gui::context_impl gui_ctx(&gui_data, false);
    gui::context = &gui_ctx;

    gui::menu_impl menu_ctx;
    gui::menu = &menu_ctx;

    hook_holder all_hooks(hooked::d3d9_reset({ d3d_ifc, 16 }),
                          hooked::d3d9_present({ d3d_ifc, 17 }),
                          hooked::wndproc(hwnd, GetWindowLongPtrW(hwnd, GWLP_WNDPROC)),
                          hooked::lock_cursor({ (void*)0, 67 }));

    if (!all_hooks.enable())
        _Fail();

    std::set_terminate([] {
        if (ResumeThread(_T_Handle) == -1)
            std::abort();
    });

    /*if (!library_info::_Wait(L"serverbrowser.dll"))
        _Fail();*/

    if (SuspendThread(_T_Handle) == -1)
        _Fail();

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
