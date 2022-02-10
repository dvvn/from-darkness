module;

#include "cheat/service/basic_includes.h"
#include "cheat/csgo/modules_includes.h"
#include "cheat/netvars/storage_includes.h"

#include <functional>

module cheat.players:list;
import cheat.netvars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

players_list::players_list( ) = default;
players_list::~players_list( ) = default;

void players_list::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
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

static float _Lerp_time(const ICVar * cvars)
{
	const auto lerp_amount = cvars->FindVar<"cl_interp">( )->get<float>( );
	auto lerp_ratio = cvars->FindVar<"cl_interp_ratio">( )->get<float>( );

	if (lerp_ratio == 0.0f)
		lerp_ratio = 1.0f;

	const auto min_ratio = cvars->FindVar<"sv_client_min_interp_ratio">( )->get<float>( );
	if (min_ratio != -1.0f)
	{
		const auto max_ratio = cvars->FindVar<"sv_client_max_interp_ratio">( )->get<float>( );
		lerp_ratio = std::clamp(lerp_ratio, min_ratio, max_ratio);
	}

	const auto update_rate = std::clamp(cvars->FindVar<"cl_updaterate">( )->get<float>( ), cvars->FindVar<"sv_minupdaterate">( )->get<float>( ), cvars->FindVar<"sv_maxupdaterate">( )->get<float>( ));
	return std::max(lerp_amount, lerp_ratio / update_rate);
}

static float _Correct_value(IVEngineClient * engine, const ICVar * cvars)
{
	const auto nci = engine->GetNetChannelInfo( );
	const auto latency = nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING);
	const auto lerp = _Lerp_time(cvars);
	const auto unlag_limit = cvars->FindVar<"sv_maxunlag">( )->get<float>( );
	return std::clamp(latency + lerp, 0.f, unlag_limit);
}

void players_list::update( )
{
	const auto& interfaces = this->deps( ).get<csgo_interfaces>( );

	const auto globals = interfaces.global_vars.get( );
	const auto max_clients = static_cast<size_t>(globals->max_clients);
	storage_.resize(max_clients);

	const auto engine = interfaces.engine.get( );
	const auto cvars = interfaces.cvars.get( );
	const auto update_players =
		[this,
		ents_list = interfaces.entity_list.get( ),
#ifdef _DEBUG
		frametime = globals->frametime,
#endif
		curtime = globals->curtime,//todo: made it fixed
		correct = _Correct_value(engine, cvars)
		]<class Fn>(size_t start, const Fn validator, size_t limit)
	{
		for (auto i = start; std::invoke(validator, i, limit); ++i)
		{
			auto& entry = storage_[i];
			auto end = static_cast<C_CSPlayer*>(ents_list->GetClientEntity(i));
			entry.update(end, curtime, correct);
#ifdef _DEBUG
			if (!entry.team.ghost)
			{
				_Draw_server_hitboxes(i, frametime, false);
			}
#endif
		}
	};

	const auto local_idx = engine->GetLocalPlayer( );

	update_players(1, std::less( ), local_idx);
	storage_[local_idx].update(nullptr, 0, 0);
	update_players(local_idx + 1, std::less_equal( ), max_clients);
}
