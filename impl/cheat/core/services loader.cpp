#include "services loader.h"
#include "console.h"
#ifndef CHEAT_GUI_TEST
#include "csgo_awaiter.h"
#endif

#include <dhooks/hook_utils.h>

#include <nstd/os/module info.h>
#include <nstd/os/threads.h>

#include <robin_hood.h>
#include <cppcoro/sync_wait.hpp>

#include <Windows.h>

#ifndef CHEAT_GUI_TEST
#include <thread>
#endif
#include <functional>
using namespace cheat;
using namespace detail;

#ifndef CHEAT_GUI_TEST

struct services_loader::load_thread : std::jthread
{
	load_thread() = default;

	load_thread& operator=(std::jthread&& thr)
	{
		*static_cast<std::jthread*>(this) = std::move(thr);
		return *this;
	}
};

#endif

struct unload_helper_data
{
	DWORD sleep;
	HMODULE handle;
	BOOL retval;
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
		for (auto& s : base.services( ))
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
	for (auto& h : all_hooks)
		h->disable( );

	frozen.clear( );
	holder->reset( ); //destroy all except hooks
	Sleep(sleep / 2);
	all_hooks.clear( ); //unhook all
	Sleep(sleep / 2);
	//we must be close all threads before unload!
	FreeLibraryAndExitThread(handle, retval);
}

#ifndef CHEAT_GUI_TEST

HMODULE services_loader::my_handle() const
{
	return my_handle__;
}

void services_loader::load(HMODULE handle)
{
	my_handle__   = handle;
	*load_thread_ = std::jthread([this]
	{
		auto ex = make_executor( );

		const auto load_helper = [&]
		{
			return cppcoro::sync_wait(this->load(ex));
		};

		if (!load_helper( ))
			this->unload( );
		else
			csgo_awaiter::get_ptr( )->reset( );
	});
}

void services_loader::unload()
{
	this->load_thread_->request_stop( );

	const auto data = new unload_helper_data;
	data->sleep     = 1000;
	data->handle    = my_handle__;
	data->retval    = TRUE;
	data->holder    = this;

	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

std::stop_token services_loader::load_thread_stop_token() const
{
	return load_thread_->get_stop_token( );
}
#endif

service_base::executor services_loader::make_executor()
{
	return executor(/*std::min<size_t>(8, std::thread::hardware_concurrency( ))*/);
}

service_base::load_result services_loader::load_impl() noexcept
{
	CHEAT_CONSOLE_LOG("Cheat fully loaded");
	co_return (true);
}

services_loader::~services_loader()
{
}

//template <typename Where, typename Obj>
//static void _Add_service( )
//{
//	Where::get_ptr( )->template wait_for_service<Obj>( );
//}

services_loader::services_loader()
{
#ifndef CHEAT_GUI_TEST
	load_thread_ = std::make_unique<load_thread>( );
#endif
	//using namespace hooks;
	//using namespace gui;

	//WARNING!
	//true async is possible only with a very large number of threads!
	//next example freeze program
	//if we have X services waiting for another Y out-of-range other services, and this number lower than thread pool size, the programm will freeze! I didn't add a workaround for this

#ifndef CHEAT_GUI_TEST
	///this->wait_for_service<csgo_awaiter>( );
#endif
	///this->wait_for_service<console>( ); //await: csgo_awaiter
	///this->wait_for_service<csgo_interfaces>( );                               //await: console
	//this->wait_for_service<settings>( );                                      //
	///this->wait_for_service<imgui_context>( );                                 //await: csgo_interfaces
	///this->wait_for_service<menu>( );                                          //await: imgui_context
	///this->wait_for_service<vgui_surface::lock_cursor>( );                     //await: menu
	///this->wait_for_service<netvars>( );                                       //await: csgo_interfaces
	///this->wait_for_service<players_list>( );                                  //await: netvars
	///this->wait_for_service<winapi::wndproc>( );                               //await: imgui_context
	///this->wait_for_service<directx::present>( );                              //await: menu
	///this->wait_for_service<directx::reset>( );                                //await: imgui_context
	///this->wait_for_service<c_base_entity::estimate_abs_velocity>( );          //await: netvars
	///this->wait_for_service<c_base_entity::should_interpolate>( );             //await: netvars
	///this->wait_for_service<c_base_animating::should_skip_animation_frame>( ); //await: netvars
	///this->wait_for_service<c_base_animating::standard_blending_rules>( );     //await: netvars
	///this->wait_for_service<c_csplayer::do_extra_bone_processing>( );          //await: csgo_interfaces
	///this->wait_for_service<client::frame_stage_notify>( );                    //await: players_list
	///this->wait_for_service<client_mode::create_move>( );                      //await: players_list
	///this->wait_for_service<studio_render::draw_model>( );                     //await: players_list

	//#ifndef CHEAT_GUI_TEST
	//	_Add_service<console, csgo_awaiter>( );
	//#endif
	//	_Add_service<csgo_interfaces, console>( );
	//
	//	_Add_service<imgui_context, csgo_interfaces>( );
	//	_Add_service<netvars, csgo_interfaces>( );
	//	_Add_service<c_csplayer::do_extra_bone_processing, csgo_interfaces>( );
	//
	//	_Add_service<menu, imgui_context>( );
	//	_Add_service<winapi::wndproc, imgui_context>( );
	//	_Add_service<directx::reset, imgui_context>( );
	//
	//	_Add_service<vgui_surface::lock_cursor, menu>( );
	//	_Add_service<directx::present, menu>( );
	//
	//	_Add_service<players_list, netvars>( );
	//	_Add_service<c_base_entity::estimate_abs_velocity, netvars>( );
	//	_Add_service<c_base_entity::should_interpolate, netvars>( );
	//	_Add_service<c_base_animating::should_skip_animation_frame, netvars>( );
	//	_Add_service<c_base_animating::standard_blending_rules, netvars>( );
	//
	//	_Add_service<client::frame_stage_notify, players_list>( );
	//	_Add_service<client_mode::create_move, players_list>( );
	//	_Add_service<studio_render::draw_model, players_list>( );
}
