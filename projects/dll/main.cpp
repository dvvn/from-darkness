#include "console.h"

#include <fd/gui/context.h>
#include <fd/gui/menu.h>
#include <fd/hooking/callback.h>
#include <fd/hooking/storage.h>
#include <fd/library_info.h>
#include <fd/netvars/storage.h>
#include <fd/utils/functional.h>

#include <fd/valve/base_client.h>
#include <fd/valve/cs_player.h>
#include <fd/valve/engine_client.h>
#include <fd/valve/gui/surface.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <d3d9.h>
#include <windows.h>

#include <iostream>

static HMODULE _ModuleHandle;

static HANDLE _ThreadHandle;
static DWORD  _ThreadId = 0;

#if 0
static void _exit_fail()
{
    // TerminateThread(_ThreadHandle, EXIT_FAILURE);
    // FreeLibrary(ModuleHandle);
    FreeLibraryAndExitThread(_ModuleHandle, EXIT_FAILURE);
}

static void _exit_success()
{
    FreeLibraryAndExitThread(_ModuleHandle, EXIT_SUCCESS);
}
#endif

namespace fd
{

#if 0
template <size_t S>
struct system_library_name
{
    wchar_t buffer[S + 4];

    consteval system_library_name(const char* name)
        : buffer()
    {
        auto libNameSize = _size(buffer) - 4;
        for (size_t i = 0; i != libNameSize; ++i)
            buffer[i] = to_lower(name[i]);
        _copy(".dll", 4, buffer + libNameSize);
    }

    auto begin() const
    {
        return buffer;
    }

    auto end() const
    {
        return buffer + _size(buffer);
    }

    operator wstring_view() const
    {
        return { begin(), end() };
    }
};

template <size_t S>
system_library_name(const char (&name)[S]) -> system_library_name<S - 1>;

#define CSGO_LIB(_NAME_) csgo_library_info_ex _NAME_ = this->wait(system_library_name(#_NAME_ ".dll"))

struct csgo_libs_storage : library_info_cache
{
    CSGO_LIB(server);
    CSGO_LIB(client);
    CSGO_LIB(engine);
    CSGO_LIB(dataCache);
    CSGO_LIB(materialSystem);
    CSGO_LIB(vstdlib);
    CSGO_LIB(vgui2);
    CSGO_LIB(vguiMatSurface);
    CSGO_LIB(vphysics);
    CSGO_LIB(inputSystem);
    CSGO_LIB(studioRender);
    CSGO_LIB(shaderApiDx9);
    CSGO_LIB(serverBrowser);
};

#undef CSGO_LIB
#endif

class csgo_library_info_ex : public csgo_library_info
{
    void* createInterfaceFn_;

    using csgo_library_info::find_interface;

  public:
    csgo_library_info_ex(library_info info)
        : csgo_library_info(info)
        , createInterfaceFn_(info.find_export("CreateInterface"))
    {
    }

    auto find_interface(std::string_view name) const
    {
        return this->find_interface(createInterfaceFn_, name);
    }
};

template <class T>
concept have_init_fn = requires() { T::init(); };

template <class T>
static void _init_netvars()
{
    if constexpr (have_init_fn<T>)
        T::init();
}

static auto _get_product_version_string(valve::engine_client* engine)
{
    auto nativeStr = std::string_view(engine->GetProductVersionString());

    std::wstring buff;
    buff.reserve(nativeStr.size());
    for (auto c : nativeStr)
        buff += c == '.' ? '_' : c;
    return buff;
}

static netvars_storage* _NetvarsStorage;

basic_netvars_storage* get_netvars_storage()
{
    return _NetvarsStorage;
}

static void _unload()
{
    if (ResumeThread(_ThreadHandle) == -1)
        abort(); // WARNING!!!
}

static void _pause()
{
    if (SuspendThread(_ThreadHandle) == -1)
        abort(); // WARNING!!!
}

static DWORD WINAPI _context(void*) noexcept
{
    DWORD              exitCode   = EXIT_FAILURE;
    invoke_on_destruct freeHelper = [&]
    {
        FreeLibraryAndExitThread(_ModuleHandle, exitCode);
    };

#ifdef _DEBUG
    auto consoleHanlder = console_holder(
        L"from-darkness debug console. " BOOST_STRINGIZE(__DATE__) " " BOOST_STRINGIZE(__TIME__));
#endif

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
#else
    spdlog::set_level(spdlog::level::warning);
#endif

    set_current_library(_ModuleHandle);
    library_info_cache libs;

    csgo_library_info_ex  shaderApiLib = libs.get(L"shaderapidx9.dll");
    IDirect3DDevice9*&    d3dIfc       = *(shaderApiLib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
    csgo_library_info_ex  clientLib    = libs.get(L"client.dll");
    valve::base_client*   gameClient   = clientLib.find_interface("VClient");
    csgo_library_info_ex  engineLib    = libs.get(L"engine.dll");
    valve::engine_client* gameEngine   = engineLib.find_interface("VEngineClient");
    valve::cs_player*&    localPlayer  = *(clientLib.find_signature("A1 ? ? ? ? 89 45 BC 85 C0") + 1);

    // addToSafeList
    clientLib.find_signature("56 8B 71 3C B8").as_fn<void(__fastcall*)(HMODULE, void*)>(_ModuleHandle, nullptr);

    netvars_storage netvarsStorage;
    _NetvarsStorage = &netvarsStorage;

    netvarsStorage.iterate_client_class(gameClient->GetAllClasses());
#if 0 // WIP
    if (localPlayer)
    {
        netvarsStorage.iterate_datamap(localPlayer->GetDataDescMap());
        netvarsStorage.iterate_datamap(localPlayer->GetPredictionDescMap());
    }
    else
    {
        auto vtable = static_cast<valve::cs_player*>(clientLib.find_vtable("C_CSPlayer"));
        netvarsStorage.iterate_datamap(vtable->GetDataDescMap());
        netvarsStorage.iterate_datamap(vtable->GetPredictionDescMap());
    }
#endif

#ifdef _DEBUG
    netvars_classes lazyNetvarClasses;
#ifdef FD_WORK_DIR
    lazyNetvarClasses.dir.append(BOOST_STRINGIZE(FD_WORK_DIR)).append("netvars_generated");
#else
#error "provide directory for netvars_classes"
#endif
    netvarsStorage.generate_classes(lazyNetvarClasses);
    netvars_log lazyNetvarLog;
#ifdef FD_ROOT_DIR
    lazyNetvarLog.dir.append(BOOST_STRINGIZE(FD_ROOT_DIR)).append(".dumps/netvars");
#else
#error "provide directory for netvars_log"
#endif
    lazyNetvarLog.file.name      = _get_product_version_string(gameEngine);
    lazyNetvarLog.file.extension = L".json";
    lazyNetvarLog.indent         = 4;
    lazyNetvarLog.filler         = ' ';
    netvarsStorage.log_netvars(lazyNetvarLog);
#endif

    _init_netvars<valve::cs_player>();
    netvarsStorage.clear();

    auto hackMenu = menu(tab_bar(
        tab("tab",
            []
            {
                ImGui::TextUnformatted("test");
                if (ImGui::Button("Unload"))
                    _unload();
            })));

    while (!d3dIfc)
        Sleep(10);
    D3DDEVICE_CREATION_PARAMETERS d3dParams;
    if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
        abort(); // WARNING!!!

    gui_context guiCtx(
        { false, d3dIfc, d3dParams.hFocusWindow },
        [&]
        {
            [[maybe_unused]] auto visible = hackMenu.render();
#ifndef IMGUI_DISABLE_DEMO_WINDOWS
            if (visible)
                ImGui::ShowDemoWindow();
#endif
        });
    if (!guiCtx)
        return FALSE;
    invoke_on_destruct guiCtxProtector = [&]
    {
        if (!d3dIfc)
            guiCtx.detach();
    };

    csgo_library_info_ex vguiLib = libs.get(L"vguimatsurface.dll");

    auto vguiSurface = static_cast<valve::gui::surface*>(vguiLib.find_interface("VGUI_Surface"));

    auto allHooks = hooks_storage(
        hook_callback_lazy(
            "WinAPI.WndProc",
            DefWindowProcW,
            decay_fn(GetWindowLongPtrW(d3dParams.hFocusWindow, GWLP_WNDPROC)),
            [&](auto orig, HWND currHwnd, auto... args) -> LRESULT
            {
                assert(currHwnd == d3dParams.hFocusWindow);
                using keys_return = basic_gui_context::keys_return;
                switch (guiCtx.process_keys(currHwnd, args...))
                {
                case keys_return::instant:
                    return TRUE;
                case keys_return::native:
                    return orig(currHwnd, args...);
                case keys_return::def:
                    return DefWindowProcW(currHwnd, args...);
                default:
                    std::unreachable();
                }
            }),
        hook_callback_lazy(
            "IDirect3DDevice9::Reset",
            &IDirect3DDevice9::Reset,
            decay_fn(d3dIfc, 16),
            [&](auto orig, auto, auto... args)
            {
                guiCtx.release_textures();
                return orig(args...);
            }),
        hook_callback_lazy(
            "IDirect3DDevice9::Present",
            &IDirect3DDevice9::Present,
            decay_fn(d3dIfc, 17),
            [&](auto orig, auto thisPtr, auto... args)
            {
                guiCtx.render(thisPtr);
                return orig(args...);
            }),
        hook_callback_lazy(
            "VGUI.ISurface::LockCursor",
            &valve::gui::surface::LockCursor,
            decay_fn(vguiSurface, 67),
            [&](auto orig, auto thisPtr)
            {
                if (hackMenu.visible() && !thisPtr->IsCursorVisible())
                {
                    thisPtr->UnlockCursor();
                    return;
                }
                orig();
            }));

    if (!allHooks.enable())
        return FALSE;

#if 0
    if (!libs.wait(L"serverbrowser.dll"))
        _exit_fail();
#endif

    _pause();

    if (!allHooks.disable())
        return FALSE;

    Sleep(100);

    exitCode = EXIT_SUCCESS;
    return TRUE;
}
} // namespace fd

// ReSharper disable once CppInconsistentNaming
BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID /*reserved*/)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    {
        if (!DisableThreadLibraryCalls(moduleHandle))
            return FALSE;
        _ModuleHandle = moduleHandle;
        _ThreadHandle = CreateThread(nullptr, 0, fd::_context, nullptr, 0, &_ThreadId);
        if (!_ThreadId)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        // if (_ThreadId && WaitForSingleObject(_ThreadHandle, INFINITE) == WAIT_FAILED)
        //     return FALSE;
        break;
    }
    }

    return TRUE;
}