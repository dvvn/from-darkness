#include "player.h"
#include "players_list.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/sdk/CClientState.hpp"
#include "cheat/sdk/IClientEntityList.hpp"
#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"
#include "cheat/utils/game.h"

#include "nstd/memory backup.h"
#include "nstd/runtime assert.h"

#include <nstd/enum_tools.h>

#if CHEAT_FEATURE_PLAYER_LIST
#include <excpt.h>
#endif

using namespace cheat;
using namespace detail;
using namespace csgo;

#if 0

void player_shared_impl::init([[maybe_unused]] C_CSPlayer* owner)
{
#if !CHEAT_FEATURE_PLAYER_LIST
	CHEAT_CALL_BLOCKER
#else
	shared_holder::init( );
	const auto pl = this->get( );

	pl->sim_time = /*ent->m_flSimulationTime( )*/-1;
	pl->dormant  = owner->IsDormant( );
	//this->index = ent->EntIndex( );
	pl->ent   = owner;
	pl->alive = owner->IsAlive( );
	pl->team  = static_cast<m_iTeamNum_t>(owner->m_iTeamNum( ));
	pl->ticks.reserve(player::max_ticks_count( ));
	owner->m_bClientSideAnimation( ) = false;
	this->destroy_fn_                = [](const player& p)
	{
		__try
		{
			p.ent->m_bClientSideAnimation( ) = true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	};
#endif
}

bool player_shared_impl::update_simtime()
{
	//todo: tickbase shift workaround
#if !CHEAT_FEATURE_PLAYER_LIST
	CHEAT_CALL_BLOCKER
#else
	auto& p = **this;

	if (const auto new_sim_time = p.ent->m_flSimulationTime( ); p.sim_time < new_sim_time)
	{
		p.sim_time = new_sim_time;
		return true;
	}
#endif
	return false;
}

void player_shared_impl::update_animations([[maybe_unused]] bool simple)
{
#if !CHEAT_FEATURE_PLAYER_LIST
	CHEAT_CALL_BLOCKER
#else

	(void)this;

	const auto p = this->get( )->ent;

	const auto backup_layers = [p]
	{
		auto& layers = p->GetAnimOverlays( );
		runtime_assert(layers.size( ) == 13);
		auto& bytes_array = *reinterpret_cast<std::array<uint8_t, sizeof(CAnimationLayer) * 13>*>(layers.begin( ));
		return nstd::memory_backup(bytes_array);
	}( );
	(void)backup_layers;

	if (simple)
	{
		//--
		p->UpdateClientSideAnimation( );
	}
	else
	{
		//todo: proper animfix
		p->UpdateClientSideAnimation( );
	}
#endif
}

void player_shared_impl::store_tick()
{
	auto& data = this->share( );
	auto& t    = data->ticks.emplace_front( );

	t.init(*data);

	(void)this;
}

void player_shared_impl::remove_old_ticks(float curtime)
{
	auto& pl           = *share( );
	auto& ticks        = pl.ticks;
	auto& ticks_window = pl.ticks_window;

	const auto erase_uselles = [&]
	{
		const auto limit = player::max_ticks_count( );
		if (ticks.size( ) <= limit)
			return;

		//dont erase any !valid ticks here, in theory we can increase (fake) ping and hit these

		ticks.erase(std::next(ticks.begin( ), limit), ticks.end( ));
	};

	const auto update_window = [&]
	{
		const auto begin = ticks.begin( );
		const auto end   = ticks.end( );

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto first_valid = [&]
		{
			for (auto itr = begin; itr != end; ++itr)
			{
				if (itr->share( )->is_valid(curtime))
					return itr;
			}

			return end;
		}( );

		if (first_valid == end)
		{
			ticks_window = {};
			return;
		}

		// ReSharper disable once CppTooWideScopeInitStatement
		const auto last_valid = [&]
		{
			for (auto itr = std::prev(end); itr != first_valid; --itr)
			{
				if (itr->share( )->is_valid(curtime))
					return itr;
			}

			return end;
		}( );

		if (last_valid == end)
			ticks_window = std::span(first_valid, 1);
		else
			ticks_window = std::span(first_valid, last_valid);
	};

	(void)this;

	erase_uselles( );
	update_window( );
}

#endif

size_t player::max_ticks_count()
{
	return utils::time_to_ticks(utils::unlag_limit( ) + utils::unlag_range( ));
}

player::team_info::team_info(m_iTeamNum_t val)
	: value(val)
{
	switch (val)
	{
		case m_iTeamNum_t::UNKNOWN:
		case m_iTeamNum_t::SPEC:
		{
			enemy = false;
			ghost = true;
			break;
		}
		case m_iTeamNum_t::T:
		case m_iTeamNum_t::CT:
		{
			const auto local      = csgo_interfaces::get_ptr( )->local_player.get( );
			const auto local_team = static_cast<m_iTeamNum_t>(local->m_iTeamNum( ));

			enemy = local_team != val;
			break;
		}
	}
}

player::team_info::team_info(std::underlying_type_t<m_iTeamNum_t> val)
	: team_info(static_cast<m_iTeamNum_t>(val))
{
}

//todo: bool return only when wanted (othervise void)

void player::update(int index)
{
	const auto ent = static_cast<C_CSPlayer*>(csgo_interfaces::get_ptr( )->entity_list->GetClientEntity(index));

	CHEAT_PLAYER_PROP_FN_INVOKE(entptr, ent);
	if (!entptr)
		return;
	CHEAT_PLAYER_PROP_FN_INVOKE(simtime, ent->m_flSimulationTime( ));
	if (tick.updated == update_state::IDLE)
		return;
	CHEAT_PLAYER_PROP_FN_INVOKE(team, ent->m_iTeamNum( ));
	CHEAT_PLAYER_PROP_FN_INVOKE(health, ent->m_iHealth( ));
	CHEAT_PLAYER_PROP_FN_INVOKE(dormant, ent->IsDormant( ));
	CHEAT_PLAYER_PROP_FN_INVOKE(dmgprotect, ent->m_bGunGameImmunity( ));

	if (team.ghost || !team.enemy)
	{
		//todo: clear ticks
	}
	else
	{
	}
}

CHEAT_PLAYER_PROP_CPP(entptr)
{
	if (entptr_new == entptr)
		return;

	auto new_player = player( );

	if (entptr_new != nullptr)
	{
		new_player.local  = (entptr_new == csgo_interfaces::get_ptr( )->local_player.get( ));
		new_player.entptr = entptr_new;
	}

	*this = std::move(new_player);
}

CHEAT_PLAYER_PROP_CPP(simtime)
{
	const auto diff = simtime_new - simtime;

	if (diff == 0)
	{
		tick.updated = update_state::IDLE;
	}
	else if (diff < 0)
	{
		tick.updated = update_state::SILENT;
		//auto ticks_shifted=tick.client.current-tick.server.current;
	}
	else
	{
		tick.updated = update_state::NORMAL;

		tick.server.set(csgo_interfaces::get_ptr( )->client_state->ClockDriftMgr.nServerTick);
		tick.client.set(utils::time_to_ticks(simtime_new));

		simtime = simtime_new;
	}
}

CHEAT_PLAYER_PROP_CPP(team)
{
	if (team_new == team)
		return;

	//nothing do here, all team base stuff called after all props updated

	team = team_new;
}

CHEAT_PLAYER_PROP_CPP(health)
{
	if (health_new == health)
		return;

	health = health_new;

	if (health_new <= 0)
		team.ghost = true;
}

CHEAT_PLAYER_PROP_CPP(dormant)
{
	//simtime not changed while dormant??

	if (dormant_new == dormant)
		return;

	//const auto dormant_in  = !dormant && dormant_new;
	//const auto dormant_out = dormant && !dormant_new;

	if (dormant_new)
	{
		//todo: check can we hit here
		tick.updated = update_state::IDLE;
	}
	else if (dormant)
	{
		//dormant out
		tick.updated = update_state::SILENT;
	}

	dormant = dormant_new;
}

CHEAT_PLAYER_PROP_CPP(dmgprotect)
{
	//checked here because can be changed in dormant function
	if (tick.updated == update_state::IDLE)
		return;

	//some cheats disable aa while frozen/protected, so we can backtrack to it
	//

	if (dmgprotect_new)
		team.ghost = true;

	dmgprotect = dmgprotect_new;
}
