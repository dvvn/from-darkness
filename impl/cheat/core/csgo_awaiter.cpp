#include "csgo_awaiter.h"
#include "console.h"
#include "services_loader.h"

#ifndef CHEAT_GUI_TEST
#include <nstd/os/module info.h>
#endif

#include <cppcoro/task.hpp>

#ifndef CHEAT_GUI_TEST
#include <filesystem>
#include <functional>
#include <thread>
#endif

using namespace cheat;

auto csgo_awaiter_impl::load_impl( ) noexcept -> load_result
{
	using nstd::os::module_info;
	using nstd::os::all_modules;
	using std::filesystem::path;

	const auto modules = all_modules::get_ptr( );
	modules->update(false);

	auto work_dir         = path(modules->owner( ).work_dir( ));
	auto& work_dir_native = const_cast<path::string_type&>(work_dir.native( ));
	std::ranges::transform(work_dir_native, work_dir_native.begin( ), towlower);
	work_dir.append(L"bin").append(L"serverbrowser.dll");

	const auto is_game_loaded = [&]
	{
		return modules->rfind([&](const module_info& info)
		{
			return info.full_path( ) == work_dir.native( );
		}) != nullptr;
	};

	if (is_game_loaded( ))
	{
		game_loaded_before = true;
		CHEAT_SERVICE_LOADED
	}

	do
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for((100ms));
		//todo: cppcoro::cancellation_token
		//todo: co_yield
		if (services_loader::get_ptr( )->load_thread_stop_token( ).stop_requested( ))
			CHEAT_SERVICE_NOT_LOADED("Loading thread stopped");

		modules->update(true);

		if (is_game_loaded( ))
			CHEAT_SERVICE_LOADED
	}
	while (true);
}

CHEAT_SERVICE_REGISTER(csgo_awaiter);
