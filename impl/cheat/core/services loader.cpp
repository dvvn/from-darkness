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

using namespace cheat;
using namespace detail;
using namespace utl;

static bool _Wait_for_game( )
{
#ifdef CHEAT_GUI_TEST

	return false;

#else

	const auto modules = all_modules::get_ptr( );
	auto&      all     = modules->update(false).all( );

	auto  work_dir        = std::filesystem::path(modules->owner( ).work_dir( ));
	auto& work_dir_native = const_cast<std::filesystem::path::string_type&>(work_dir.native( ));
	ranges::transform(work_dir_native, work_dir_native.begin( ), towlower);
	work_dir.append(L"bin").append(L"serverbrowser.dll");

	const auto is_game_loaded = [&]
	{
		for (const auto& path: all | ranges::views::transform(&module_info::full_path) | ranges::views::reverse)
		{
			if (path == work_dir_native)
				return true;
		}

		return false;
	};

	if (is_game_loaded( ))
		return true;

	do
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(all.size( ) < 120 ? 1000 : 100));
		/*if (interruption_requested( ))
			throw thread_interrupted( );*/
		if (services_loader::get_ptr( )->unloaded( ))
			return false;

		modules->update(true);

		if (is_game_loaded( ))
			return false;
	}
	while (true);

#endif
}

struct unload_helper_data
{
	DWORD            sleep;
	HMODULE          handle;
	BOOL             retval;
	services_holder* instance;
};

[[maybe_unused]]
static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr                          = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval, instance] = *data_ptr;
	delete data_ptr;

	const auto hooks  = instance->get_all_hooks( );
	auto       frozen = winapi::frozen_threads_storage(true);
	for (auto& h: hooks)
	{
		h->disable( );
	}
	frozen.clear( );

	//we must be close all threads before unload!
	Sleep(sleep);
	FreeLibraryAndExitThread(handle, retval);
}

std::future<bool> services_holder::load( )
{
	return std::async(std::launch::async, [this]
	{
		bool game_alredy_loaded;
		try
		{
			game_alredy_loaded = _Wait_for_game( );
		}
		catch (const std::exception& ex)
		{
			runtime_assert(ex.what());
			return false;
		}

#ifdef NDEBUG
		std::this_thread::sleep_for(std::chrono::seconds(game_alredy_loaded ? 1 : 5));
#endif
		auto frozen = winapi::frozen_threads_storage(game_alredy_loaded);
		auto loader = thread_pool( );

		const auto sumbit = [&](const services_storage_type& services)
		{
			for (const auto& s: services)
			{
				loader.submit(std::bind_front(&service_base::load, s.get( )));
			}
		};
		const auto wait = [&](const services_storage_type& services)
		{
			do
			{
				size_t done = 0;
				using state = service_state::value_type;
				for (const auto& s: services)
				{
					switch (s->state( ).value( ))
					{
						case state::unset:
						case state::loading:
							std::this_thread::yield( );
							break;
						case state::stopped:
						case state::error:
							loader.paused = true;
						case state::loaded:
						case state::skipped:
							++done;
							break;
					}
				}

				if (done == services.size( ))
					break;
			}
			while (true);

			return loader.paused == false;
		};

		for (auto child = this; child != nullptr; child = child->next__.get( ))
		{
			sumbit(child->services_dont_wait__);
			sumbit(child->services__);

			if (!child->services_wait_for__.empty( ))
			{
				if (!wait(child->services_wait_for__))
					return false;

				child->services_wait_for__.clear( ); //it uselles now
			}

			if (!wait(child->services__))
				return false;
		}

		//wait for all services
		loader.wait_for_tasks();

		for (auto child = this; child != nullptr; child = child->next__.get( ))
		{
			//check detached if any service have error
			for (const auto& s: child->services_dont_wait__)
			{
				if (!s->state( ).done( ))
					return false;
			}
		}

		return true;
	});
}

using namespace hooks;

services_holder& services_holder::then( )
{
	if (!next__)
		next__ = std::make_unique<services_holder>( );

	return *next__;
}

nstd::unordered_set<hook_holder_base*> services_holder::get_all_hooks( )
{
	auto hooks = nstd::unordered_set<hook_holder_base*>( );

	const auto extract_hooks = [&](const services_storage_type& where)
	{
		for (auto& item: where)
		{
			const auto ptr = item.get( );
			if (const auto hook = dynamic_cast<hook_holder_base*>(ptr); hook != nullptr)
				hooks.emplace(hook);
		}
	};

	for (auto child = this; child != nullptr; child = child->next__.get( ))
	{
		extract_hooks(child->services__);
		extract_hooks(child->services_dont_wait__);
	}
	return hooks;
}

std::string_view services_loader::name( ) const
{
	return _Type_name<services_loader>(false);
}

#ifndef CHEAT_GUI_TEST

HMODULE services_loader::my_handle( ) const
{
	return my_handle__;
}

void services_loader::load(HMODULE handle)
{
	my_handle__ = handle;
	std::thread([this] { this->load( ); }).detach( );
}

void services_loader::unload( )
{
	runtime_assert(this->unloaded_==false);

	const auto data = new unload_helper_data;
	data->sleep     = 1000;
	data->handle    = my_handle__;
	data->retval    = TRUE;
	data->instance  = std::addressof(this->services__);
	this->unloaded_ = true;
	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

bool services_loader::unloaded( ) const
{
	return unloaded_;
}

#endif

void services_loader::reset( )
{
	auto dummy = services_holder( );
	std::swap(services__, dummy);
}

bool services_loader::Do_load( )
{
	bool result;
	try
	{
		result = services__.load( ).get( );
	}
	catch (const std::exception& ex)
	{
		CHEAT_CONSOLE_LOG("Error while task loading: {}", ex.what());
		result = false;
	}

	if (!result)
	{
#ifndef CHEAT_GUI_TEST
		this->unload( );
#endif
	}
	else
	{
		CHEAT_CONSOLE_LOG("Cheat fully loaded");
	}

	return result;
}

services_loader::services_loader( )
{
	services__.load<console>( )
			  .then( )
			  .load<csgo_interfaces, settings, gui::menu>( )
			  .then( )
			  .load<gui::imgui_context>( )
			  .load<vgui_surface::lock_cursor>(true)
			  .load<netvars, players_list>(true)
			  .then( )
			  .load<hooks::winapi::wndproc>(true)
			  .then( )
			  .wait<netvars, players_list>( )
			  .load<c_base_entity::estimate_abs_velocity,
					c_base_entity::should_interpolate,
					c_base_animating::should_skip_animation_frame,
					c_base_animating::standard_blending_rules,
					c_csplayer::do_extra_bone_processing>(true)
			  .load<directx::present, directx::reset>(true)
			  .load<client::frame_stage_notify>( )
			  .then( )
			  .load<client_mode::create_move, studio_render::draw_model>(true);
}
