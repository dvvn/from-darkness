module;
#include "cheat/service/includes.h"
#include <nstd/rtlib/info_includes.h>
#include <thread>
#include <filesystem>

module cheat.csgo.awaiter;
import cheat.console;
import nstd.rtlib.all_infos;

using namespace cheat;
using namespace nstd::rtlib;
namespace fs = std::filesystem;

auto csgo_awaiter::load_impl( ) noexcept -> load_result
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
		co_return console::get( ).on_service_loaded(this);
	}

	do
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for((100ms));
		//todo: cppcoro::cancellation_token
		//todo: co_yield
		if (services_loader::get_ptr( )->load_thread_stop_token( ).stop_requested( ))
			co_return console::get( ).on_service_loaded<false>(this, "Loading thread stopped");

		modules->update(true);

		if (is_game_loaded( ))
			co_return console::get( ).on_service_loaded(this);

	}
	while (true);
}

//CHEAT_SERVICE_REGISTER(csgo_awaiter);
