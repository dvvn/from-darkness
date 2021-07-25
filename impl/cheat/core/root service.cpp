#include "root service.h"
#include "console.h"

#include "cheat/hooks/client mode/create move.h"
#include "cheat/hooks/c_baseanimating/should skip animation frame.h"
#include "cheat/hooks/c_baseanimating/standard blending rules.h"
#include "cheat/hooks/c_base_entity/estimate abs velocity.h"
#include "cheat/hooks/c_csplayer/do extra bone processing.h"
#include "cheat/hooks/directx/present.h"

#include "cheat/utils/winapi/threads.h"

using namespace cheat;
using namespace detail;
using namespace hooks;
using namespace utl;
using namespace winapi;

root_service::root_service( )
{
	this->Wait_for<console>( );
	this->Wait_for<directx::present>( );
	this->Wait_for<client_mode::create_move>( );

	this->Wait_for<c_csplayer::do_extra_bone_processing>( );
	this->Wait_for<c_base_animating::should_skip_animation_frame>( );
	this->Wait_for<c_base_animating::standard_blending_rules>( );
	this->Wait_for<c_base_entity::estimate_abs_velocity>( );
}

#ifdef CHEAT_GUI_TEST

void root_service::init( )
{
	auto frozen = frozen_threads_storage(true);
	auto loader = loader_type( );
	service_base::init(loader).wait( );
}

#else

static future<bool> _Wait_for_game( )
{
	return async(launch::async, []
	{
		auto& modules = all_modules::get( );
		auto& all = modules.update(false).all( );

		auto work_dir = filesystem::path(modules.owner( ).work_dir( ));
		auto& work_dir_native = const_cast<filesystem::path::string_type&>(work_dir.native( ));
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
			this_thread::sleep_for(chrono::milliseconds(all.size( ) < 120 ? 1000 : 100));
			if (this_thread::interruption_requested( ))
				throw thread_interrupted( );

			modules.update(true);

			if (is_game_loaded( ))
				return false;
		}
		while (true);
	});
}

void root_service::init(HMODULE handle)
{
	my_handle__ = handle;

	_Wait_for_game( ).then([this](const future<bool>& task1)
	{
		if (task1.has_exception( ))
		{
			BOOST_ASSERT("Error in modules loader!");
			this->unload(FALSE);
			return;
		}

		//game loaded, freeze all threads from there
		const auto game_alredy_loaded = *task1.future_->result;
#ifdef NDEBUG
		this_thread::sleep_for(chrono::seconds(game_alredy_loaded ? 1 : 5));
#endif
		auto frozen = frozen_threads_storage(game_alredy_loaded);
		auto loader = make_unique<loader_type>( );

		static_cast<service_base*>(this)->
				init(*loader).
				then(launch::async, [frozen_holder = move(frozen), loader_holder = move(loader), this](const load_task_type& task2) mutable
				{
					loader_holder.reset( ); //destroy thread pool and wait until all threads stop
					if (!task2.has_exception( ))
					{
#ifdef CHEAT_HAVE_CONSOLE
						console::get_ptr( )->write_line("Cheat fully loaded");
#endif
					}
					else
					{
						BOOST_ASSERT("Error while loading!");
						this->unload(FALSE);
					}
				});
	});
}

struct unload_helper_data
{
	DWORD sleep;
	HMODULE handle;
	BOOL retval;
	service_base* instance;

	function<service_base::wait_for_storage_type*(service_base*)> storage_getter;
};

static DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval, instance, storage_getter] = *data_ptr;
	delete data_ptr;

	auto hooks = unordered_set<hook_holder_base*>( );

	const function<void(service_base*)> get_all_hooks = [&](service_base* from)
	{
		auto storage = storage_getter(from);
		if (!storage)
			return;
		for (auto& item: *storage)
		{
			const auto ptr = item.get( );
			if (const auto hook = dynamic_cast<hook_holder_base*>(ptr); hook != nullptr)
				hooks.emplace(hook);

			get_all_hooks(ptr);
		}
	};

	get_all_hooks(instance);

	auto frozen = frozen_threads_storage(true);
	for (auto& h: hooks)
	{
		h->disable_safe( );
	}
	frozen.clear( );

	//we must be close all threads before unload!
	Sleep(sleep);
	FreeLibraryAndExitThread(handle, retval);
}

void root_service::unload(BOOL ret)
{
	const auto data = new unload_helper_data;
	data->sleep = 1000;
	data->handle = my_handle__;
	data->retval = ret;
	data->instance = this;
	data->storage_getter = [](service_base* s)-> wait_for_storage_type*
	{
		return s->wait_for__.empty( ) ? nullptr : addressof(s->wait_for__);
	};

	CreateThread(nullptr, 0, _Unload_helper, data, 0, nullptr);
}

HMODULE root_service::my_handle( ) const
{
	return my_handle__;
}

#endif

void root_service::Load( )
{
	(void)this;
}
