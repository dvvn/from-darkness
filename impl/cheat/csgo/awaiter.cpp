module;
#include "cheat/service/includes.h"
#include <nstd/rtlib/includes.h>

#include <filesystem>

module cheat.csgo.awaiter;
import cheat.console;
import nstd.rtlib;

using namespace cheat;
using namespace nstd::rtlib;
namespace fs = std::filesystem;

#ifndef CHEAT_GUI_TEST

bool csgo_awaiter::load_impl( ) noexcept
{
	const auto modules = all_infos::get_ptr( );
	modules->update(false);

	fs::path work_dir = modules->owner( ).work_dir( );
	auto& work_dir_native = const_cast<fs::path::string_type&>(work_dir.native( ));
	std::ranges::transform(work_dir_native, work_dir_native.begin( ), towlower);
	work_dir.append(L"bin").append(L"serverbrowser.dll");

	const auto is_game_loaded = [&]
	{
		return modules->rfind([&](const info& i)
							  {
								  return i.full_path( ) == work_dir.native( );
							  }) != nullptr;
	};

	if (is_game_loaded( ))
	{
		game_loaded_before = true;
		return true;
	}

	do
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for((100ms));
		//todo: cppcoro::cancellation_token
		if (services_loader::get_ptr( )->load_thread_stop_token( ).stop_requested( ))
			return false;

		modules->update(true);

		if (is_game_loaded( ))
			return true;

	}
	while (true);
}

CHEAT_SERVICE_REGISTER(csgo_awaiter);
#endif