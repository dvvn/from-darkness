#ifdef _DEBUG
#include <fd/assert_impl.h>
#include <fd/logger_impl.h>
#include <fd/system_console.h>
#endif

#include <fd/functional.h>
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_impl.h>
#include <fd/library_info.h>

#include <fd/valve/gui/surface.h>

#include <d3d9.h>
#include <windows.h>

static HMODULE _ModuleHandle;

static HANDLE _ThreadHandle;
static DWORD _ThreadId = 0;

using namespace fd;

class hooks_storage final : public hook_global_callback
{
    std::vector<basic_hook*> hooks_;

    void construct(basic_hook* caller) override
    {
        hooks_.push_back(caller);
    }

    void destroy(const basic_hook* caller, const bool unhooked) override
    {
        if (!unhooked)
            suspend();
        // std::ranges stuck here
        *std::find(hooks_.begin(), hooks_.end(), caller) = nullptr;
    }

  public:
    ~hooks_storage() override
    {
        if (!hooks_.empty())
            suspend();
    }

    bool enable() const
    {
        return std::ranges::all_of(hooks_, &basic_hook::enable);
    }
};

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

    CurrentLibraryHandle = _ModuleHandle;

#if defined(_DEBUG) || defined(_RELEASE_DEBUG)
    system_console sysConsole;

    default_logs_handler logsCallback;
    Logger = &logsCallback;

    logsCallback.add([&](auto msg) {
        sysConsole.write(msg);
    });
#endif
#ifdef _DEBUG
    default_assert_handler assertCallback;
    AssertHandler = &assertCallback;

    assertCallback.add([&](auto& adata) {
        sysConsole.write(parse_assert_data(adata));
    });
#endif

    const library_info clientLib(L"client.dll", true);
    const auto addToSafeList = reinterpret_cast<void(__fastcall*)(HMODULE, void*)>(clientLib.find_signature("56 8B 71 3C B8"));

    invoke(addToSafeList, _ModuleHandle, nullptr);

    const auto d3dIfc = [] {
        const library_info lib(L"shaderapidx9.dll", true);
        const auto addr = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        return **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3dParams;
        if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
            _exit_fail();
        return d3dParams.hFocusWindow;
    }();

    gui::menu menu;
    gui::context guiCtx(d3dIfc, hwnd, false);
    guiCtx.store([&] {
        menu.render(&guiCtx);
    });
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
    guiCtx.store([&] {
        if (menu.visible())
            ImGui::ShowDemoWindow();
    });
#endif

    hooks_storage allHooks;
    HookGlobalCallback = &allHooks;

    hook_callback_t<WNDPROC> hkWndProc(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    hkWndProc.set_name("WinAPI.WndProc");
#ifdef _DEBUG
    hkWndProc.add([&assertCallback, hwnd](auto&, auto&, bool& interrupt, const HWND currHwnd, auto...) {
        if (currHwnd == hwnd)
            return;
        invoke(assertCallback, assert_data(nullptr, "Incorrect HWND!"));
        interrupt = true;
    });
#endif
    hkWndProc.add([&](auto&, auto& ret, bool, auto... args) {
        switch (guiCtx.process_keys(args...))
        {
        case gui::process_keys_result::instant:
            ret.emplace(TRUE);
            break;
        case gui::process_keys_result::native:
            return;
        case gui::process_keys_result::def:
            ret.emplace(DefWindowProcW(args...));
            break;
        default:
            unreachable();
        }
    });

    hook_callback hkDirectx9Reset(&IDirect3DDevice9::Reset, { d3dIfc, 16 });
    hkDirectx9Reset.set_name("IDirect3DDevice9::Reset");
    hkDirectx9Reset.add([&](auto&&...) {
        guiCtx.release_textures();
    });

    hook_callback hkDirectx9Present(&IDirect3DDevice9::Present, { d3dIfc, 17 });
    hkDirectx9Present.set_name("IDirect3DDevice9::Present");
    hkDirectx9Present.add([&](auto&, auto&, bool, auto thisPtr, auto...) {
        guiCtx.render(thisPtr);
    });

    hook_callback hkVguiLockCursor(&valve::gui::surface::LockCursor, { (void*)0, 67 });
    hkVguiLockCursor.set_name("VGUI.ISurface::LockCursor");
    hkVguiLockCursor.add([&](auto&, auto& ret, bool, auto thisPtr) {
        if (menu.visible() && !thisPtr->IsCursorVisible())
        {
            thisPtr->UnlockCursor();
            ret.emplace();
        }
    });

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

    // disable all hooks before return

    _exit_success();
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