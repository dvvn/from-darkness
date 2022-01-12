module;
#include "cheat/service/includes.h"
#include <nstd/rtlib/includes.h>

#include <filesystem>

module cheat.csgo.awaiter;
import nstd.rtlib;

using namespace cheat;
using namespace nstd::rtlib;

#ifndef CHEAT_GUI_TEST

namespace fs = std::filesystem;

template<class T>
struct fs_hash :std::hash<typename T::string_type>
{
	auto operator()(const T& p)const
	{
		return std::invoke(*this, p.native( ));
	}
};

void csgo_awaiter::load_async( ) noexcept
{
}

bool csgo_awaiter::load_impl( ) noexcept
{
	auto& modules = all_infos::get( );
	modules.update(false);

	const auto& work_dir0 = modules.owner( ).work_dir( ).fixed;
	fs::path work_dir = {work_dir0.begin( ),work_dir0.end( )};
	work_dir.append(L"bin").append(L"serverbrowser.dll");
	const nstd::hashed_wstring_view work_dir_hash = work_dir.native( );

	const auto is_game_loaded = [&]
	{
		auto modules_r = modules | std::views::reverse;
		auto itr = std::ranges::find_if(modules_r, [&](const info& i) {return work_dir_hash == i.full_path( ).fixed; });
		return itr != modules_r.end( );
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

		modules.update(true);

		if (is_game_loaded( ))
			return true;

	}
	while (true);
}
#endif

CHEAT_SERVICE_REGISTER_GAME(csgo_awaiter);