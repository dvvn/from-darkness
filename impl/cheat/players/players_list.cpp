#include "players_list.h"
#include "player.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/csgo/GlobalVars.hpp"
#include "cheat/csgo/IVEngineClient.hpp"
#include "cheat/netvars/netvars.h"
#include "cheat/utils/game.h"

#include <cppcoro/task.hpp>
#include <nstd/address_pipe.h>
#include <vector>

using namespace cheat;
using namespace detail;
using namespace csgo;

auto players_list_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_SERVICE_LOADED;
}

players_list_impl::players_list_impl( )
{
	this->add_dependency(netvars::get( ));
}

players_list_impl::~players_list_impl( ) = default;

static void* _Player_by_index_server(int client_index)
{
	static auto fn = csgo_modules::server->find_signature("85 C9 7E 32 A1").cast<void* (__fastcall*)(int)>( );
	return fn(client_index);
}

static void _Draw_server_hitboxes(int client_index, float duration, bool use_mono_color)
{
	static auto fn = csgo_modules::server->find_signature("E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF")
										  .jmp(1)
										  .cast<void(__vectorcall*)(void*, uintptr_t, float, float, float, bool)>( );
	const auto player = _Player_by_index_server(client_index);
	return fn(player, 0u, 0.f, duration, 0.f, use_mono_color);
}

void players_list_impl::update( )
{
	const auto& interfaces = csgo_interfaces::get( );
	// ReSharper disable once CppUseStructuredBinding
	const auto& globals = *interfaces->global_vars.get( );

	const auto max_clients = globals.max_clients;
	storage_.resize(max_clients);

	const auto curtime = globals.curtime; //todo: made it fixed
	const auto correct = [&]
	{
		const auto engine = interfaces->engine.get( );
		const auto nci    = engine->GetNetChannelInfo( );

		return std::clamp(nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING) + utils::lerp_time( ), 0.f, utils::unlag_limit( ));
	}( );

	for (auto i = static_cast<decltype(CGlobalVarsBase::max_clients)>(1u); i <= max_clients; ++i)
	{
		auto& entry = storage_[i - 1];
		entry.update(i, curtime, correct);

#ifdef _DEBUG
		if (!entry.team.ghost && !entry.local)
		{
			_Draw_server_hitboxes(i, globals.frametime, false);
		}
#endif
	}
}

CHEAT_SERVICE_REGISTER(players_list);
