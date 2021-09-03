#include "services loader.h"

#include "console.h"
#include "csgo interfaces.h"

#include "cheat/gui/imgui context.h"
#include "cheat/gui/menu.h"
#include "cheat/hooks/client mode/create move.h"
#include "cheat/hooks/client/frame stage notify.h"
#include "cheat/hooks/c_baseanimating/should skip animation frame.h"
#include "cheat/hooks/c_baseanimating/standard blending rules.h"
#include "cheat/hooks/c_base_entity/estimate abs velocity.h"
#include "cheat/hooks/c_base_entity/should interpolate.h"
#include "cheat/hooks/c_csplayer/do extra bone processing.h"
#include "cheat/hooks/directx/present.h"
#include "cheat/hooks/directx/reset.h"
#include "cheat/hooks/studio render/draw model.h"
#include "cheat/hooks/vgui surface/lock cursor.h"
#include "cheat/hooks/winapi/wndproc.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/settings/settings.h"

#include "nstd/os/module info.h"
#include "nstd/os/threads.h"

#include <robin_hood.h>

#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>

#include <Windows.h>

#include <functional>
#include <thread>

using namespace cheat;
using namespace detail;

#ifndef CHEAT_GUI_TEST

struct csgo_awaiter final: service<csgo_awaiter>
{
protected:
	load_result load_impl( ) override
	{
		const auto modules = nstd::os::all_modules::get_ptr( );
		modules->update(false);

		auto  work_dir        = std::filesystem::path(modules->owner( ).work_dir( ));
		auto& work_dir_native = const_cast<std::filesystem::path::string_type&>(work_dir.native( ));
		std::ranges::transform(work_dir_native, work_dir_native.begin( ), towlower);
		work_dir.append(L"bin").append(L"serverbrowser.dll");

		const auto is_game_loaded = [&]
		{
			return modules->rfind([&](const nstd::os::module_info& info)
			{
				return info.full_path( ) == work_dir;
			}) != nullptr;
		};

		if (is_game_loaded( ))
		{
			game_loaded_before_ = true;
			co_return service_state::loaded;
		}

		do
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			//todo: cppcoro::cancellation_token
			//todo: co_yield
			if (services_loader::get_ptr( )->load_thread_stop_token( ).stop_requested( ))
				co_return service_state::error;

			modules->update(true);

			if (is_game_loaded( ))
				co_return service_state::loaded;
		}
		while (true);
	}

public:
	bool game_loaded_before( ) const
	{
		return game_loaded_before_;
	}

protected:
	//todo:fix
	//now this stop threadpool and main thread
	/*void after_load( ) override
	{
		if (!game_loaded_before_)
			return;
		frozen_threads_.fill( );
	}

	void after_reset( ) override
	{
		if (!game_loaded_before_)
			return;
		frozen_threads_.clear( );
	}*/

private:
	bool game_loaded_before_ = false;
	//nstd::os::frozen_threads_storage frozen_threads_{false};
};

struct services_loader::load_thread: std::jthread
{
	load_thread( ) = default;

	load_thread& operator=(std::jthread&& thr)
	{
		*static_cast<std::jthread*>(this) = std::move(thr);
		return *this;
	}
};

#endif

struct unload_helper_data
{
	DWORD            sleep;
	HMODULE          handle;
	BOOL             retval;
	services_loader* holder;
};

[[maybe_unused]]
static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr                        = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval, holder] = *data_ptr;
	delete data_ptr;

	using hooks_storage = robin_hood::unordered_set<std::shared_ptr<dhooks::hook_holder_base>>;
	using get_all_hooks_fn = std::function<void(const service_base& base, hooks_storage& set)>;
	const get_all_hooks_fn get_all_hooks = [&](const service_base& base, hooks_storage& set)
	{
		for (auto& s: base.services( ))
		{
			auto ptr = std::dynamic_pointer_cast<dhooks::hook_holder_base>(s);
			if (ptr != nullptr)
				set.emplace(std::move(ptr));

			get_all_hooks(*s, set);
		}
	};

	auto all_hooks = hooks_storage( );
	get_all_hooks(*holder, all_hooks);

	auto frozen = nstd::os::frozen_threads_storage(true);
	for (auto& h: all_hooks)
		h->disable( );

	frozen.clear( );
	all_hooks.clear( );
	holder->reset( );

	//we must be close all threads before unload!
	Sleep(sleep);
	FreeLibraryAndExitThread(handle, retval);
}

#ifndef CHEAT_GUI_TEST

HMODULE services_loader::my_handle( ) const
{
	return my_handle__;
}

void services_loader::load(HMODULE handle)
{
	my_handle__   = handle;
	*load_thread_ = std::jthread([this]
	{
		auto ex = executor(std::max<size_t>(8, std::thread::hardware_concurrency( )));
		if (sync_wait(this->load(ex)) != service_state::loaded)
			this->unload( );
		else
			csgo_awaiter::get_ptr( )->reset( );
	});
}

void services_loader::unload( )
{
	this->load_thread_->request_stop( );

	const auto data = new unload_helper_data;
	data->sleep     = 1000;
	data->handle    = my_handle__;
	data->retval    = TRUE;
	data->holder    = this;

	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

std::stop_token services_loader::load_thread_stop_token( ) const
{
#ifndef CHEAT_GUI_TEST
	return load_thread_->get_stop_token( );
#else
	return {};
#endif
}

#endif

service_base::load_result services_loader::load_impl( )
{
	co_return service_state::loaded;
}

void services_loader::after_load( )
{
	CHEAT_CONSOLE_LOG("Cheat fully loaded");
}

services_loader::~services_loader( )
{
}

template <typename Where, typename Obj>
static void _Add_service( )
{
	Where::get_ptr( )->template add_service<Obj>( );
}

services_loader::services_loader( )
{
#ifndef CHEAT_GUI_TEST
	load_thread_ = std::make_unique<load_thread>( );
#endif
	using namespace hooks;
	using namespace gui;

	//WARNING!
	//true async is possible only with a very large number of threads!
	//next example freeze program
	//if we have X services waiting for another Y out-of-range other services, and this number lower than thread pool size, the programm will freeze! I didn't add a workaround for this

#ifndef CHEAT_GUI_TEST
	this->add_service<csgo_awaiter>( );
#endif
	this->add_service<console>( );                                       //await: csgo_awaiter
	this->add_service<csgo_interfaces>( );                               //await: console
	this->add_service<settings>( );                                      //
	this->add_service<imgui_context>( );                                 //await: csgo_interfaces
	this->add_service<menu>( );                                          //await: imgui_context
	this->add_service<vgui_surface::lock_cursor>( );                     //await: menu
	this->add_service<netvars>( );                                       //await: csgo_interfaces
	this->add_service<players_list>( );                                  //await: netvars
	this->add_service<winapi::wndproc>( );                               //await: imgui_context
	this->add_service<directx::present>( );                              //await: menu
	this->add_service<directx::reset>( );                                //await: imgui_context
	this->add_service<c_base_entity::estimate_abs_velocity>( );          //await: netvars
	this->add_service<c_base_entity::should_interpolate>( );             //await: netvars
	this->add_service<c_base_animating::should_skip_animation_frame>( ); //await: netvars
	this->add_service<c_base_animating::standard_blending_rules>( );     //await: netvars
	this->add_service<c_csplayer::do_extra_bone_processing>( );          //await: csgo_interfaces
	this->add_service<client::frame_stage_notify>( );                    //await: players_list
	this->add_service<client_mode::create_move>( );                      //await: players_list
	this->add_service<studio_render::draw_model>( );                     //await: players_list

#ifndef CHEAT_GUI_TEST
	_Add_service<console, csgo_awaiter>( );
#endif
	_Add_service<csgo_interfaces, console>( );

	_Add_service<imgui_context, csgo_interfaces>( );
	_Add_service<netvars, csgo_interfaces>( );
	_Add_service<c_csplayer::do_extra_bone_processing, csgo_interfaces>( );

	_Add_service<menu, imgui_context>( );
	_Add_service<winapi::wndproc, imgui_context>( );
	_Add_service<directx::reset, imgui_context>( );

	_Add_service<vgui_surface::lock_cursor, menu>( );
	_Add_service<directx::present, menu>( );

	_Add_service<players_list, netvars>( );
	_Add_service<c_base_entity::estimate_abs_velocity, netvars>( );
	_Add_service<c_base_entity::should_interpolate, netvars>( );
	_Add_service<c_base_animating::should_skip_animation_frame, netvars>( );
	_Add_service<c_base_animating::standard_blending_rules, netvars>( );

	_Add_service<client::frame_stage_notify, players_list>( );
	_Add_service<client_mode::create_move, players_list>( );
	_Add_service<studio_render::draw_model, players_list>( );
}
