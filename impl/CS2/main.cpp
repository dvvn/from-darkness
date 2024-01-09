#include "entity_cache/holder.h"
#include "functional/vfunc.h"
#include "hook/backend/minhook.h"
#include "hook/creator.h"
#include "hooked/directx11.h"
#include "hooked/game_entity_system.h"
#include "hooked/winapi.h"
#include "library_info/native/engine.h"
#include "library_info/native/schema_system.h"
#include "library_info/native/tier0.h"
#include "winapi/window_info.h"
//
#include "core/basic_dll_context.h"
#include "gui/native_data.h"
#include "menu_example.h"

namespace fd
{
inline class : public basic_context, public basic_dll_context
{
  protected:
    [[no_unique_address]] basic_context_data_holder<gui::native_data_dx11> gui_data;

  public:
    bool run()
    {
        auto&& logger             = this->debug_logger.get();
        auto const FD_RANDOM_NAME = logger.make_notification();

        auto gui_data = this->gui_data.get();

        auto menu = make_menu_example([=] {
            if (!this->resume())
                unreachable();
        });

        entity_cache ent_cache;

        auto const [cvar_system]            = tier0_dll{}.obj();
        auto const [engine, game_resources] = engine2_dll{}.obj();
        auto const [schema_system]          = schema_system_dll{}.obj();

        hook_backend_minhook hook_backend;
        create_hook_helper const hook_creator{&hook_backend};

        auto const& render_data = gui_data.render_backend.data();
        hooked::DXGI_factory::create_swap_chain hk_create_swap_chain{&gui_data.render_backend};
        if (!hook_creator(vfunc{&IDXGIFactory::CreateSwapChain, render_data.DXGI_factory()}, &hk_create_swap_chain))
            return false;
        hooked::DXGI_swap_chain::resize_buffers hk_resize_buffers{&gui_data.render_backend};
        if (!hook_creator(vfunc{&IDXGISwapChain::ResizeBuffers, render_data.swap_chain()}, &hk_resize_buffers))
            return false;
        hooked::DXGI_swap_chain::present hk_present{[&] {
            gui_data.present(&menu);
        }};
        if (!hook_creator(vfunc{&IDXGISwapChain::Present, render_data.swap_chain()}, &hk_present))
            return false;
        win::window_info const main_window{gui_data.system_backend.window()};
        hooked::winapi::wndproc hk_wndproc{&gui_data.system_backend};
        if (!hook_creator(main_window.proc(), &hk_wndproc))
            return false;
        using cache_entity_vfunc = vfunc<void* (native::game_resource_service::*)(native::entity_instance*, native::CBaseHandle)>;
        hooked::game_entity_system::on_add_entity hk_on_add_entity{&ent_cache};
        if (!hook_creator(cache_entity_vfunc{14, game_resources}, &hk_on_add_entity))
            return false;
        hooked::game_entity_system::on_remove_entity hk_on_remove_entity{&ent_cache};
        if (!hook_creator(cache_entity_vfunc{15, game_resources}, &hk_on_remove_entity))
            return false;

        if (!hook_backend.enable())
            return false;

        logger("Loaded");

        if (!this->pause())
            return false;

        return true;
    }
} dll_context;

basic_dll_context* get_dll_context()
{
    return &dll_context;
}

size_t get_dll_context_stack_size()
{
    return 0; // not implemented
}

bool attach_context()
{
    return dll_context.run();
}
} // namespace fd
