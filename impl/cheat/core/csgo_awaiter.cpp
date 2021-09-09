#include "csgo_awaiter.h"

#include "services loader.h"

#include <nstd/os/module info.h>

#include <filesystem>
// ReSharper disable once CppUnusedIncludeDirective
#include <functional>
#include <thread>

using namespace cheat;

service_base::load_result csgo_awaiter::load_impl( )
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

csgo_awaiter::csgo_awaiter( )
	: service_maybe_skipped
	(
#ifdef CHEAT_GUI_TEST
	 true
#else
	false
#endif
	)
{
}

bool csgo_awaiter::game_loaded_before( ) const
{
	return game_loaded_before_;
}

CHEAT_REGISTER_SERVICE(csgo_awaiter);
