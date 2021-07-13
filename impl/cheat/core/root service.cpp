#include "root service.h"

#include "console.h"

#include "cheat/hooks/client mode/create move.h"
#include "cheat/hooks/client/frame stage notify.h"
#include "cheat/hooks/directx/present.h"
#include "cheat/hooks/directx/reset.h"

#include "cheat/utils/winapi/threads.h"

using namespace cheat;
using namespace hooks;
using namespace utl;
using namespace winapi;

root_service::root_service( )
{
#ifdef CHEAT_DEBUG_MODE
	this->Wait_for<console>( );
#endif
	this->Wait_for<directx::reset>( );
	this->Wait_for<directx::present>( );
	this->Wait_for<client::frame_stage_notify>( );
	this->Wait_for<client_mode::create_move>( );
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

		auto  work_dir = filesystem::path(modules.owner( ).work_dir( ));
		auto& work_dir_native = const_cast<filesystem::path::string_type&>(work_dir.native( ));
		ranges::transform(work_dir_native, work_dir_native.begin( ), towlower);
		work_dir.append(_T("bin")).append(_T("serverbrowser.dll"));

		auto first_time = true;
		do
		{
			if (all.size( ) >= 160)
			{
				for (const auto& path: all | ranges::views::transform(&module_info::full_path) | ranges::views::reverse)
				{
					if (path == work_dir_native)
						return first_time;
				}
			}

			this_thread::sleep_for(chrono::milliseconds(100));
			if (this_thread::interruption_requested( ))
				throw thread_interrupted( );

			first_time = false;
			modules.update(true);
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
		auto frozen = frozen_threads_storage(!task1.future_.get( ));
		auto loader = make_unique<loader_type>( );

		static_cast<service_base*>(this)->
				init(*loader).
				then(launch::async, [frozen_holder = move(frozen), loader_holder = move(loader), this](const load_task_type& task2) mutable
				{
					loader_holder.reset( ); //destroy thread pool and wait for until all threads stop
					if (!task2.has_exception( ))
					{
#ifdef CHEAT_DEBUG_MODE
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
	DWORD         sleep;
	HMODULE       handle;
	BOOL          retval;
	root_service* instance;

	cheat::detail::service_base::wait_for_storage_type* storage;
};

DWORD WINAPI _Unload_helper(LPVOID data_packed)
{
	const auto data_ptr = static_cast<unload_helper_data*>(data_packed);
	const auto [sleep, handle, retval, instance, storage] = *data_ptr;
	delete data_ptr;

	auto frozen = frozen_threads_storage(true);

	for (auto& item: *storage)
	{
		if (const auto hook = dynamic_cast<hook_holder_base*>(item.get( )); hook != nullptr)
			hook->disable_safe( );
	}

	//we must be close all threads before unload!
	//console::get( ).finish( );
	frozen.clear( );
	Sleep(sleep);
	FreeLibraryAndExitThread(handle, retval);
}

void root_service::unload(BOOL ret)
{
	const auto data = new unload_helper_data; 
	data->sleep = 1500;
	data->handle = my_handle__;
	data->retval = ret;
	data->storage = addressof(this->Storage( ));
	data->instance = this;

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
