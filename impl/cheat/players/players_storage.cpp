module;

#include <functional>
#include <algorithm>
#include <vector>

module cheat.players;
import cheat.csgo.modules;
import cheat.csgo.interfaces.ConVar;
import cheat.csgo.interfaces.EngineClient;
import cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.interfaces.C_CSPlayer;
import cheat.console.lifetime_notification;
import nstd.one_instance;

using namespace cheat;
using namespace players;
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

template<typename T, class Base = std::vector<T>>
class extended_vector :Base
{
public:
	auto& at(size_t index)
	{
		return (*this)[index - 1];
	}

	const auto& at(size_t index) const
	{
		return (*this)[index - 1];
	}

	using Base::begin;
	using Base::end;
	using Base::data;
	using Base::size;
	using Base::resize;
};

struct players_storage : console::lifetime_notification<players_storage>, extended_vector<player>
{
	players_storage( ) = default;
};

std::string_view console::lifetime_notification<players_storage>::_Name( ) const
{
	return "players";
}

using players_holder = nstd::one_instance<players_storage>;

player* players::begin( )
{
	return players_holder::get( ).data( );
}

player* players::end( )
{
	auto& holder = players_holder::get( );
	return holder.data( ) + holder.size( );
}

void players::update( )
{
	const auto max_clients = static_cast<size_t>(CGlobalVarsBase::get( ).max_clients);
	players_holder::get( ).resize(max_clients);
#if 0
	const auto update_players =
		[
#ifdef _DEBUG
			frametime = CGlobalVarsBase::get( ).frametime,
#endif
			curtime = CGlobalVarsBase::get( ).curtime,//todo: made it fixed
			correct = _Correct_value( )
		]<typename Fn>(const size_t start, const Fn validator, const size_t limit)
	{

		for (auto i = start; std::invoke(validator, i, limit); ++i)
		{
			auto& entry = players_holder::get( ).at(i);
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
	players_holder::get( ).at(local_idx).update(nullptr, 0, 0);
	update_players(local_idx + 1, std::less_equal( ), max_clients);
#endif
}
