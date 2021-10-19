#include "players_list.h"
#include "player.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/csgo modules.h"
#include "cheat/core/services loader.h"

#include "cheat/csgo/entity/C_CSPlayer.h"
#include "cheat/csgo/GlobalVars.hpp"
#include "cheat/csgo/IClientEntityList.hpp"
#include "cheat/csgo/IVEngineClient.hpp"

#include "cheat/netvars/netvars.h"

#include "cheat/utils/game.h"

#include <nstd/memory backup.h>
#include <nstd/runtime_assert_fwd.h>
#include <nstd/address_pipe.h>

#include <vector>

using namespace cheat;
using namespace detail;
using namespace csgo;

service_base::load_result players_list::load_impl( ) noexcept
{
	CHEAT_SERVICE_INIT(CHEAT_FEATURE_PLAYER_LIST)
}

struct players_list::storage_type
#if CHEAT_FEATURE_PLAYER_LIST
		: std::vector<player>
#endif
{
};

players_list::players_list( )
{
	storage_ = std::make_unique<storage_type>( );
	this->wait_for_service<netvars>( );
}

players_list::~players_list( ) = default;

#if defined(_DEBUG) && CHEAT_FEATURE_PLAYER_LIST

static void* _Player_by_index_server(int client_index)
{
	using namespace nstd::address_pipe;
	static auto fn = CHEAT_FIND_SIG(server, "85 C9 7E 32 A1", cast<void* (__fastcall*)(int)>);
	return fn(client_index);
}

static void _Draw_server_hitboxes(int client_index, float duration, bool use_mono_color)
{
	using namespace nstd::address_pipe;
	static auto fn    = CHEAT_FIND_SIG(server, "E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF", jmp(1), cast<void(__vectorcall*)(void*, uintptr_t, float, float, float, bool)>);
	const auto player = _Player_by_index_server(client_index);
	return fn(player, 0u, 0.f, duration, 0.f, use_mono_color);
}
#endif

void players_list::update( )
{
#if !CHEAT_FEATURE_PLAYER_LIST
	CHEAT_CALL_BLOCKER
#else

	const auto interfaces = csgo_interfaces::get_ptr( );
	// ReSharper disable once CppUseStructuredBinding
	const auto& globals = *interfaces->global_vars.get( );

	const auto max_clients = globals.max_clients;
	storage_->resize(max_clients);

	const auto curtime = globals.curtime; //todo: made it fixed
	const auto correct = [&]
	{
		const auto engine = interfaces->engine.get( );
		const auto nci    = engine->GetNetChannelInfo( );

		return std::clamp(nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING) + utils::lerp_time( ), 0.f, utils::unlag_limit( ));
	}( );

	for (size_t i = 1; i <= max_clients; ++i)
	{
		auto& entry = (*storage_)[i - 1];
		entry.update(i, curtime, correct);

#ifdef _DEBUG
		if (!entry.team.ghost && !entry.local)
		{
			_Draw_server_hitboxes(i, globals.frametime, false);
		}
#endif
	}

#endif
}

CHEAT_REGISTER_SERVICE(players_list);
