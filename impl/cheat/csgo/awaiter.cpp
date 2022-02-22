module;
#include "cheat/service/basic_includes.h"
#include <nstd/rtlib/includes.h>

#include <filesystem>

module cheat.csgo.awaiter;
import nstd.rtlib;

using namespace cheat;

csgo_awaiter::~csgo_awaiter( )
{
	*destroyed_ = true;
}

void csgo_awaiter::construct( ) noexcept
{
	destroyed_ = std::make_shared<std::atomic_bool>(false);
}

bool csgo_awaiter::load( ) noexcept
{
	using namespace nstd;
	using namespace rtlib;

	namespace fs = std::filesystem;

	auto& modules = all_infos::get( );
	modules.update(false);

	const auto& owner_path = modules.owner( ).work_dir( ).fixed;
	const nstd::hashed_wstring last_dll = const_cast<std::wstring&&>(
		fs::path(owner_path.begin( ), owner_path.end( )).append(L"bin").append(L"serverbrowser.dll").native( ));

	const auto is_game_loaded = [&]
	{
		for (const auto& m : modules | std::views::reverse)
		{
			if (m.full_path( ).fixed == last_dll)
				return true;
		}

		return false;
	};

	if (is_game_loaded( ))
	{
		game_loaded_before = true;
		return true;
	}

	auto destroyed = destroyed_;

	for (;;)
	{
		if (*destroyed)
			return false;

		modules.update(true);
		if (is_game_loaded( ))
			return true;

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}

}