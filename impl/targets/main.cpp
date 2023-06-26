#include "fd/tool/algorithm/pattern.h"

#include <fd/console.h>
#include <fd/entity_cache.h>
#include <fd/hook.h>
#include <fd/library_info.h>
#include <fd/log.h>
#include <fd/netvar_storage.h>
#include <fd/render/context.h>
#include <fd/render/own_backend.h>
#include <fd/tool/span.h>
#include <fd/valve/client.h>
#include <fd/valve/engine.h>
#include <fd/valve/entity.h>
#include <fd/valve/entity_handle.h>
#include <fd/valve/entity_list.h>
#include <fd/valve/global_vars.h>
#include <fd/valve/vgui.h>
#include <fd/vtable.h>

#undef interface

#define FD_SHARED_LIB

#define DECLSPEC_NAKED __declspec(naked)

static HINSTANCE self_handle;
#ifdef FD_SHARED_LIB
static HANDLE thread;
static DWORD thread_id;

[[noreturn]]
static void exit_context(bool success)
{
    assert(GetCurrentThread() == thread);
    FreeLibraryAndExitThread(self_handle, success ? EXIT_SUCCESS : EXIT_FAILURE);
}

static bool lock_context()
{
    return SuspendThread(thread) != -1;
}

static bool resume_context()
{
    return ResumeThread(thread) != -1;
}
#endif

namespace fd
{
#ifdef FD_SHARED_LIB

// RETURN_ADDRESS_GADGET(valve::IClientEntityList, client);
// RETURN_ADDRESS_GADGET(valve::CClientEntityList, client);
// RETURN_ADDRESS_GADGET(valve::CHLClient, client);
// RETURN_ADDRESS_GADGET(valve::C_BaseEntity, client);
// RETURN_ADDRESS_GADGET(valve::ISurface, vguimatsurface);
// RETURN_ADDRESS_GADGET(valve::IEngineClient, engine);
// RETURN_ADDRESS_GADGET(FD_RENDER_BACKEND, shaderapidx9);

// todo: get sendpacked from cl_move
template <typename Callback, typename C>
static void __fastcall createmove_proxy(
    C *thisptr,
    uintptr_t edx,
    int sequence_number,
    float input_sample_frametime,
    bool is_active,
    bool *send_packed)
{
    using proxy_holder = hook_proxy_member_holder<call_type_t::thiscall_, void, C, int, float, bool>;
    (*unique_hook_callback<Callback>) //
        (proxy_holder(unique_hook_trampoline<Callback>, thisptr),
         sequence_number,
         input_sample_frametime,
         is_active,
         send_packed);
}

template <typename Callback, typename C>
static DECLSPEC_NAKED void __fastcall createmove_proxy_naked(
    C *thisptr,
    uintptr_t edx,
    int sequence_number,
    float input_sample_frametime,
    bool is_active)
{
    static constexpr auto proxy = createmove_proxy<Callback, C>;

    __asm
    {
		push ebp // save register
		mov ebp, esp; // store stack to register
		push ebx; // save register
		push esp; // 'send_packet' from caller stack
		push [ebp+10h]; // 'is_active'
		push [ebp+0Ch]; // 'input_sample_frametime'
		push [ebp+8]; // 'sequence_number'
		call proxy
		pop ebx // restore register
		pop ebp // restore register
		retn 0Ch
    }
}

template <typename Callback, call_type_t Call_T, typename Ret, typename C, typename... Args>
struct hook_proxy_createmove
{
    static_assert(Call_T == call_type_t::thiscall_);
};

template <typename Callback, call_type_t Call_T, typename C, typename... Args>
struct hook_proxy_getter<hook_proxy_createmove<Callback, Call_T, C, Args...>>
{
    static void *get()
    {
        return createmove_proxy_naked<Callback, C>;
    }
};
#endif

#define NAMED_VFUNC(_NAME_, _OBJ_) BOOST_STRINGIZE(_NAME_), _OBJ_[&_NAME_]
#define NATIVE_LIB(_L_)            native_library<#_L_> _L_##_dll

static bool context() noexcept
{
#ifdef _DEBUG
#ifdef FD_SHARED_LIB
    system_console console;
    if (!console.init())
        return false;
#endif
    logging_activator log_activator;
    if (!log_activator.init())
        return false;
#endif
    vtable<FD_RENDER_BACKEND> render_vtable;
    HWND window;
    WNDPROC window_proc;

#ifdef FD_SHARED_LIB
    NATIVE_LIB(client);
    NATIVE_LIB(engine);
    NATIVE_LIB(vguimatsurface);
    NATIVE_LIB(shaderapidx9);

    render_vtable = **reinterpret_cast<IDirect3DDevice9 ***>(
        (uint8_t *)shaderapidx9_dll.pattern("A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

    D3DDEVICE_CREATION_PARAMETERS creation_parameters;
    if (FAILED(render_vtable->GetCreationParameters(&creation_parameters)))
        return false;
    window      = creation_parameters.hFocusWindow;
    window_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(creation_parameters.hFocusWindow, GWLP_WNDPROC));

#else
    own_render_backend own_render;
    if (!own_render.init(L"Unnamed", self_handle))
        return false;

    render_vtable = own_render.device.get();
    window        = own_render.hwnd;
    window_proc   = own_render.info.lpfnWndProc;
#endif

    render_context render;
    if (!render.init(window, render_vtable.instance()))
        return false;

#ifdef FD_SHARED_LIB

    native_client v_client(client_dll.interface("VClient"));
    native_engine v_engine(engine_dll.interface("VEngineClient"));
    native_gui_surface v_gui(vguimatsurface_dll.interface("VGUI_Surface"));
    native_entity_list v_ent_list(client_dll.interface("VClientEntityList"));
    auto v_globals = **static_cast<valve::global_vars_base ***>(v_client[11] + 0xA);

    native_entity_finder entity_finder(v_ent_list.get_client_entity);
    entity_cache cached_entity(&entity_finder, v_engine.in_game());

    native_entity csplayer_vtable(
        v_engine.in_game() ? entity_finder.get(v_engine.local_player_index()) : client_dll.vtable("C_CSPlayer"));

    netvar_storage netvars;
    netvars.store(v_client.get_all_classes());
    netvars.store(csplayer_vtable.get_desc_data_map());
    netvars.store(csplayer_vtable.get_prediction_data_map());
#endif

    hook_context hooks;
    hooks.create(
        "WinAPI.WndProc",
        window_proc,
        [&render,
#ifdef FD_SHARED_LIB
         def = IsWindowUnicode(window) ? DefWindowProcW : DefWindowProcA
#else
         def = DefWindowProc
#endif
    ](auto orig, auto... args) -> LRESULT {
            using result_t = render_context::process_result;
            result_t pmr;
            render.process_message(args..., &pmr);
            switch (pmr)
            {
            case result_t::idle:
                return orig(args...);
            case result_t::updated:
                return /*def*/ orig(args...); // TEMP USE orig
            case result_t::locked:
                return TRUE;
            default:
                std::unreachable();
            }
        });
    hooks.create(NAMED_VFUNC(FD_RENDER_BACKEND::Reset, render_vtable), [&](auto &&orig, auto... args) {
        render.reset();
        return orig(args...);
    });
    hooks.create(NAMED_VFUNC(FD_RENDER_BACKEND::Present, render_vtable), [&](auto &&orig, auto... args) {
        if (auto frame = render.new_frame())
        {
            ImGui::ShowDemoWindow();
            if (ImGui::Button("Exit"))
            {
#ifdef FD_SHARED_LIB
                resume_context();
#else
                own_render.stop();
#endif
            }
        }
        return orig(args...);
    });
#ifdef FD_SHARED_LIB
    hooks.create(NAMED_VFUNC(FD_RENDER_BACKEND::Release, render_vtable), [&](auto &&orig) {
        auto refs = orig();
        if (refs == 0 && orig == render_vtable)
            render.detach();
        return refs;
    });
    hooks.create(
        "CHLClient::FrameStageNotify",
        v_client[37].get<void, native_frame_stage>(),
        [&](auto &&orig, native_frame_stage stage) {
            if (v_engine.in_game())
            {
                if (stage == native_frame_stage::render_start)
                {
                    if (!cached_entity.synced())
                    {
                        cached_entity.sync(v_ent_list.max_entities());
                        cached_entity.mark_synced();
                    }
                }
            }
            orig(stage);
        });
    hooks.create("VGUI.ISurface::LockCursor", v_gui.lock_cursor, [&](auto &&orig) {
        // if (hack_menu.visible() && !this_ptr->IsCursorVisible() /*&& ifc.engine->IsInGame()*/)
        //{
        //     this_ptr->UnlockCursor();
        //     return;
        // }
        orig();
    });
    hooks.create<hook_proxy_createmove>(
        "CHLClient::CreateMove",
        v_client[22].get<std::false_type>(),
        [&](auto &&orig, int sequence_number, float input_sample_frametime, bool is_active, bool *send_packed) {
            //
            orig(sequence_number, input_sample_frametime, is_active);
        });
    using entity_list_callback = void(__thiscall *)(CClientEntityList *, void *, valve::entity_handle);
    hooks.create(
        "CClientEntityList::OnAddEntity",
        void_to_func<entity_list_callback>(client_dll.pattern("55 8B EC 51 8B 45 0C 53 56 8B F1 57")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            orig(handle_interface, handle);
            // todo: work with this_ptr
            cached_entity.add(handle.index());
        });
    hooks.create(
        "CClientEntityList::OnRemoveEntity",
        void_to_func<entity_list_callback>(client_dll.pattern("55 8B EC 51 8B 45 0C 53 8B D9 56 57 83 F8 FF 75 07")),
        [&](auto &&orig, auto handle_interface, auto handle) {
            // todo: work with this_ptr
            cached_entity.remove(handle.index());
            orig(handle_interface, handle);
        });
#endif

#if 1
    if (!hooks.enable_lazy())
        return false;
#else
    if (!hooks.enable())
        return false;
#endif

#ifdef FD_SHARED_LIB
    if (!lock_context())
        return false;
#else
    if (!own_render.run())
        return false;
#endif

#if 1
    if (!hooks.disable_lazy())
        return false;
#else
    if (!hooks.disable())
        return false;
#endif

    return true;
}
} // namespace fd

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        thread = CreateThread(
            nullptr,
            0,
            [](LPVOID ptr) -> DWORD {
                self_handle  = static_cast<HINSTANCE>(ptr);
                auto success = fd::context();
                exit_context(success);
            },
            handle,
            0,
            &thread_id);
        if (!thread)
            return FALSE;
        break;
    }
#if 0
    case DLL_THREAD_ATTACH: // Do thread-specific initialization.
        break;
    case DLL_THREAD_DETACH: // Do thread-specific cleanup.
        break;
#endif
    case DLL_PROCESS_DETACH:
        if (reserved != nullptr) // do not do cleanup if process termination scenario
        {
            break;
        }

        // Perform any necessary cleanup.
        break;
    }

    return TRUE;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    self_handle = GetModuleHandle(nullptr);
    auto result = fd::context();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}