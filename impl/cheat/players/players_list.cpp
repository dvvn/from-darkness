module;

#include "cheat/service/includes.h"
#include "cheat/csgo/modules_includes.h"
#include "cheat/netvars/storage_includes.h"

module cheat.players:list;
import cheat.netvars;
import cheat.utils.game;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

players_list::players_list( ) = default;
players_list::~players_list( ) = default;

void players_list::load_async( ) noexcept
{
	this->deps( ).add<netvars>( );
}

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

void players_list::update( )
{
	const auto& interfaces = services_loader::get( ).deps( ).get<csgo_interfaces>( );
	// ReSharper disable once CppUseStructuredBinding
	const auto& globals = *interfaces.global_vars.get( );

	const auto max_clients = globals.max_clients;
	storage_.resize(max_clients);

	const auto curtime = globals.curtime; //todo: made it fixed
	const auto correct = [&]
	{
		const auto engine = interfaces.engine.get( );
		const auto nci = engine->GetNetChannelInfo( );

		return std::clamp(nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING) + utils::lerp_time( ), 0.f, utils::unlag_limit( ));
	}();

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
