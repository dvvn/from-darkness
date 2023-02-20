#ifdef _DEBUG
#include <fd/assert_impl.h>
#endif
#include <fd/gui/context_impl.h>
#include <fd/gui/menu_impl.h>
#include <fd/hook_callback.h>
#include <fd/hook_storage.h>
#include <fd/library_info.h>
#include <fd/netvar_storage_impl.h>
#include <fd/string_info.h>
#include <fd/system_console.h>
#include <fd/utility.h>
#include <fd/valve/base_client.h>
#include <fd/valve/cs_player.h>
#include <fd/valve/engine_client.h>
#include <fd/valve/gui/surface.h>

#include <spdlog/spdlog.h>

#include <d3d9.h>
#include <windows.h>

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

using namespace fd;

struct free_helper
{
    DWORD exitCode;

    ~free_helper()
    {
        FreeLibraryAndExitThread(_ModuleHandle, exitCode);
    }
};

#if 0
template <size_t S>
struct system_library_name
{
    wchar_t buffer[S + 4];

    consteval system_library_name(const char* name)
        : buffer()
    {
        const auto libNameSize = _size(buffer) - 4;
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

    void* find_interface(string_view name) const
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
    const string_view nativeStr(engine->GetProductVersionString());

    wstring buff;
    buff.reserve(nativeStr.size());
    for (const auto c : (nativeStr))
        buff += c == '.' ? '_' : c;
    return buff;
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
    free_helper freeHelper{ EXIT_FAILURE };

    // set_unload(_exit_fail); //prefer terminate

#ifdef _DEBUG
    spdlog::set_level(spdlog::level::debug);
    const default_assert_handler assertHandler([&](const assert_data& adata) {
        spdlog::critical(parse(adata));
    });
#else
    spdlog::set_level(spdlog::level::);
#endif

    set_current_library(_ModuleHandle);
    library_info_cache libs;

    const auto d3dIfc = [&] {
        const auto lib    = libs.get(L"shaderapidx9.dll");
        const auto addr   = lib.find_signature("A1 ? ? ? ? 50 8B 08 FF 51 0C");
        auto&      result = **reinterpret_cast<IDirect3DDevice9***>(reinterpret_cast<uintptr_t>(addr) + 0x1);
        while (!result)
            Sleep(100);
        return result;
    }();

    const auto hwnd = [=] {
        D3DDEVICE_CREATION_PARAMETERS d3dParams;
        if (FAILED(d3dIfc->GetCreationParameters(&d3dParams)))
            abort(); // WARNING!!!
        return d3dParams.hFocusWindow;
    }();

    //----

    const csgo_library_info_ex clientLib = (libs.get(L"client.dll"));

    const auto addToSafeList = reinterpret_cast<void(__fastcall*)(HMODULE, void*)>(clientLib.find_signature("56 8B 71 3C B8"));
    addToSafeList(_ModuleHandle, nullptr);

    const auto gameClient = static_cast<valve::base_client*>(clientLib.find_interface("VClient"));

    //----

    const csgo_library_info_ex engineLib = libs.get(L"engine.dll");

    const auto gameEngine = static_cast<valve::engine_client*>(engineLib.find_interface("VEngineClient"));

    //----

    auto& localPlayer = [&]() -> valve::cs_player*& {
        auto addr = clientLib.find_signature("A1 ? ? ? ? 89 45 BC 85 C0");
        return **reinterpret_cast<valve::cs_player***>(reinterpret_cast<uintptr_t>(addr) + 1);
    }();

    //----

    netvars_storage netvarsStorage;
    Netvars = &netvarsStorage;

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
    write_string(lazyNetvarClasses.dir, FD_STRINGIZE(FD_WORK_DIR), "/valve/generated/");
#else
#error "provide directory for netvars_classes"
#endif
    netvarsStorage.generate_classes(lazyNetvarClasses);
    netvars_log lazyNetvarLog;
#ifdef FD_ROOT_DIR
    write_string(lazyNetvarLog.dir, FD_STRINGIZE(FD_ROOT_DIR), "/.dumps/netvars/");
#else
#error "provide directory for netvars_log"
#endif
    lazyNetvarLog.file.name      = _get_product_version_string(gameEngine);
    lazyNetvarLog.file.extension = L".json";
    lazyNetvarLog.indent         = 4;
    lazyNetvarLog.filler         = ' ';
    netvarsStorage.log_netvars(lazyNetvarLog);
#endif

    netvarsStorage.finish();
    _init_netvars<valve::cs_player>();

    gui::menu_impl menu(gui::tab_bar(
#ifndef FD_GUI_RANDOM_TAB_BAR_NAME
        "tab bar",
#endif
        gui::tab(
            "tab",
            [] {
                ImGui::TextUnformatted("test");
                if (ImGui::Button("Unload"))
                    _unload();
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
    if (!guiCtx.init(false) || !guiCtx.init(hwnd) || !guiCtx.init(d3dIfc))
        return FALSE;

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
                    std::unreachable();
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
        )
#if 0
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
#endif
    );

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

    freeHelper.exitCode = EXIT_SUCCESS;
    return TRUE;
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
        _ThreadHandle = CreateThread(nullptr, 0, _context, nullptr, 0, &_ThreadId);
        if (!_ThreadId)
            return FALSE;
        break;
    }
    case DLL_PROCESS_DETACH: {
        // if (_ThreadId && WaitForSingleObject(_ThreadHandle, INFINITE) == WAIT_FAILED)
        //     return FALSE;
        break;
    }
    }

    return TRUE;
}