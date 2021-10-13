#include "player.h"

#include "cheat/core/csgo interfaces.h"

#include "cheat/sdk/CClientState.hpp"
#include "cheat/sdk/IClientEntityList.hpp"

#include "cheat/utils/game.h"

#include <excpt.h>

#include <nstd/memory backup.h>

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

void player_shared_impl::update_ticks_window(float curtime)
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

player::~player()
{
	if (!entptr || local)
		return;

	[&]
	{
		__try
		{
			entptr->m_bClientSideAnimation( ) = true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}( );
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
			ghost = false;
			break;
		}
	}
}

player::team_info::team_info(std::underlying_type_t<m_iTeamNum_t> val)
	: team_info(static_cast<m_iTeamNum_t>(val))
{
}

void player::update(int index, float curtime, float correct)
{
	//note: if fps < server tickount, all next calculations are wrong!!!

	const auto ent = static_cast<C_CSPlayer*>(csgo_interfaces::get_ptr( )->entity_list->GetClientEntity(index));
	if (ent != entptr)
	{
		auto new_player = player( );

		if (ent != nullptr)
		{
			new_player.local  = ent == csgo_interfaces::get_ptr( )->local_player.get( );
			new_player.entptr = ent;
		}

		*this = std::move(new_player);
	}
	if (!ent || this->local)
		return;

	//-----

	const auto simtime_new  = ent->m_flSimulationTime( );
	const auto simtime_diff = simtime_new - simtime;
	if (simtime_diff == 0)
	{
		tick.updated = update_state::IDLE;
	}
	else if (simtime_diff < 0)
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

	//-----

	if (tick.updated != update_state::IDLE)
	{
		const team_info new_team = ent->m_iTeamNum( );
		if (new_team != team)
		{
			//all logic moved outside

			team = new_team;
		}

		//-----

		const auto new_health = ent->m_iHealth( );
		if (new_health != health)
		{
			if (new_health <= 0)
			{
				team.ghost = true;
				//allow ragdoll animation
				entptr->m_bClientSideAnimation( ) = true;
			}
			else if (health <= 0)
			{
				//spawned

				if (team.enemy)
					reset_ticks( );

				//block setupbones also??
				entptr->m_bClientSideAnimation( ) = false;
			}

			health = new_health;
		}

		//-----

		const auto new_dormant = ent->IsDormant( );
		if (new_dormant != dormant)
		{
			//const auto dormant_in  = !dormant && dormant_new;
			//const auto dormant_out = dormant && !dormant_new;

			if (new_dormant)
			{
				//todo: check can we hit here
				tick.updated = update_state::IDLE;
			}
			else if (dormant)
			{
				//dormant out
				tick.updated = update_state::SILENT;
			}

			dormant = new_dormant;
		}

		const auto new_dmgprotect = ent->m_bGunGameImmunity( );
		if (new_dmgprotect != dmgprotect)
		{
			//nothing here
			dmgprotect = new_dmgprotect;
		}
	}

	//-----

	const auto updated = tick.updated == update_state::NORMAL;

	if (!team.enemy)
	{
		if (updated)
		{
			update_animations(false);
		}
	}
	else
	{
		if (team.ghost)
		{
			reset_ticks( );
			return;
		}

		if (updated)
		{
			const auto update_anims_backup = nstd::memory_backup(entptr->m_bClientSideAnimation( ), true);
			//--
			auto& new_tick = store_tick( );
			update_animations(true);
			new_tick.store_bones(entptr, true);
		}

		///update ticks window
		if (!ticks.empty( ))
		{
			///erase uselles
			const auto limit = max_ticks_count( );
			//dont erase any !valid ticks here, in theory we can increase (fake) ping and remove possible valid tick
			//erase X ticks uselles from back (because we push to front)
			while (ticks.size( ) > limit)
				ticks.pop_back( );

			//if (ticks.size( ) <= limit)
			//	return;
			//ticks.erase(std::next(ticks.begin( ), limit), ticks.end( ));

			//---------

			///update window
			//todo: optimize this fn
			//prevent loop throught whole 'ticks'
			//check begin-1 end+1 from ticks_window etc

			const auto find_valid_tick = [&]<typename Itr>(Itr first, Itr last)-> std::optional<Itr>
			{
				for (auto itr = first; itr != last; ++itr)
				{
					if (itr->get( )->is_valid(curtime, correct))
						return std::make_optional(itr);
				}

				return {};
			};

			const auto first_valid = find_valid_tick(ticks.begin( ), ticks.end( ));
			if (!first_valid.has_value( ))
			{
				ticks_window = {};
				return;
			}
			const auto last_valid = find_valid_tick(ticks.rbegin( ), std::make_reverse_iterator(std::next(*first_valid)));

			if (!last_valid.has_value( ))
				ticks_window = std::span(*first_valid, 1);
			else
				ticks_window = std::span(*first_valid, std::distance(*first_valid, std::next(*last_valid).base( )) + 1);
		}
	}
}

size_t player::max_ticks_count()
{
	return utils::time_to_ticks(utils::unlag_limit( ) + utils::unlag_range( ));
}

void player::reset_ticks()
{
	if (ticks.empty( ))
		return;

	ticks.clear( );
	ticks_window = {};
}

tick_record& player::store_tick()
{
	auto& ref = ticks.emplace_front(*this).share( );
	return *ref;
}

void player::update_animations(bool backup_layers)
{
	using bytes_array = std::array<uint8_t, sizeof(AnimOverlaysArr_t)>;
	nstd::memory_backup<bytes_array> layers_backup;
	(void)layers_backup;
	if (backup_layers)
		layers_backup = reinterpret_cast<bytes_array&>(entptr->GetAnimOverlays( ));;

	entptr->UpdateClientSideAnimation( );
}
