module;

#include <functional>
#include <algorithm>

module cheat.players:list;
import cheat.csgo.modules;
import cheat.csgo.interfaces.ConVar;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.interfaces.C_CSPlayer;

using namespace cheat;
using namespace csgo;

static void* _Player_by_index_server(int client_index)
{
	static auto fn = csgo_modules::server->find_signature("85 C9 7E 32 A1").get<void* (__fastcall*)(int)>( );
	return fn(client_index);
}

static void _Draw_server_hitboxes(int client_index, float duration, bool use_mono_color)
{
	static auto fn = csgo_modules::server->find_signature("E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 84 ? ? ? ? 33 FF")
		.jmp(1)
		.get<void(__vectorcall*)(void*, uintptr_t, float, float, float, bool)>( );
	const auto player = _Player_by_index_server(client_index);
	return fn(player, 0u, 0.f, duration, 0.f, use_mono_color);
}

static float _Lerp_time( )
{
	const auto lerp_amount = ICVar::get( ).FindVar<"cl_interp">( )->get<float>( );
	auto lerp_ratio = ICVar::get( ).FindVar<"cl_interp_ratio">( )->get<float>( );

	if (lerp_ratio == 0.0f)
		lerp_ratio = 1.0f;

	const auto min_ratio = ICVar::get( ).FindVar<"sv_client_min_interp_ratio">( )->get<float>( );
	if (min_ratio != -1.0f)
	{
		const auto max_ratio = ICVar::get( ).FindVar<"sv_client_max_interp_ratio">( )->get<float>( );
		lerp_ratio = std::clamp(lerp_ratio, min_ratio, max_ratio);
	}

	const auto update_rate = std::clamp(ICVar::get( ).FindVar<"cl_updaterate">( )->get<float>( ), ICVar::get( ).FindVar<"sv_minupdaterate">( )->get<float>( ), ICVar::get( ).FindVar<"sv_maxupdaterate">( )->get<float>( ));
	return std::max(lerp_amount, lerp_ratio / update_rate);
}

static float _Correct_value( )
{
	const auto nci = IVEngineClient::get( ).GetNetChannelInfo( );
	const auto latency = nci->GetLatency(FLOW::INCOMING) + nci->GetLatency(FLOW::OUTGOING);
	const auto lerp = _Lerp_time( );
	const auto unlag_limit = ICVar::get( ).FindVar<"sv_maxunlag">( )->get<float>( );
	return std::clamp(latency + lerp, 0.f, unlag_limit);
}

void players_list::update( )
{
	const auto max_clients = static_cast<size_t>(CGlobalVarsBase::get( ).max_clients);
	this->resize(max_clients);

	const auto update_players =
		[this,
#ifdef _DEBUG
		frametime = CGlobalVarsBase::get( ).frametime,
#endif
		curtime = CGlobalVarsBase::get( ).curtime,//todo: made it fixed
		correct = _Correct_value( )
		]<typename Fn>(const size_t start, const Fn validator, const size_t limit)
	{
		for (auto i = start; std::invoke(validator, i, limit); ++i)
		{
			auto& entry = this->at(i);
			const auto ent = static_cast<C_CSPlayer*>(IClientEntityList::get( ).GetClientEntity(i));
			entry.update(ent, curtime, correct);
#ifdef _DEBUG
			if (!entry.team.ghost)
			{
				_Draw_server_hitboxes(i, frametime, false);
			}
#endif
		}
	};

	const auto local_idx = IVEngineClient::get( ).GetLocalPlayer( );

	update_players(1, std::less( ), local_idx);
	this->at(local_idx).update(nullptr, 0, 0);
	update_players(local_idx + 1, std::less_equal( ), max_clients);
}
