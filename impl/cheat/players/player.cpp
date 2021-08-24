#include "player.h"

#include "cheat/sdk/IConVar.hpp"
#include "cheat/sdk/entity/C_CSPlayer.h"
#include "cheat/utils/game.h"

using namespace cheat;
using namespace detail;
using namespace csgo;

void player_shared_impl::init([[maybe_unused]] C_CSPlayer* owner)
{
	shared_holder::init( );

	const auto pl = this->get( );

	pl->sim_time = /*ent->m_flSimulationTime( )*/-1;
	pl->dormant  = owner->IsDormant( );
	//this->index = ent->EntIndex( );
	pl->ent   = owner;
	pl->alive = owner->IsAlive( );
	pl->team  = owner->m_iTeamNum( );
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
}

bool player_shared_impl::update_simtime( )
{
	//todo: tickbase shift workaround

	auto& p = **this;

	if (const auto new_sim_time = p.ent->m_flSimulationTime( ); p.sim_time < new_sim_time)
	{
		p.sim_time = new_sim_time;
		return true;
	}
	return false;
}

void player_shared_impl::update_animations(bool simple)
{
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
		p->UpdateClientSideAnimation( );
	}
	else
	{
		//todo: proper animfix
		p->UpdateClientSideAnimation( );
	}
}

void player_shared_impl::store_tick( )
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

		//dont remove any !valid ticks here, in theory we can increase (fake) ping and hit these

		ticks.erase(std::next(ticks.begin( ), limit), ticks.end( ));
	};

	const auto update_window = [&]
	{
		const auto begin = ticks.begin( );
		const auto end   = ticks.end( );

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
			ticks_window = { };
			return;
		}

		const auto last_valid = [&]
		{
			for (auto itr = std::prev(end); itr != first_valid; --itr)
			{
				if (itr->share( )->is_valid(curtime))
					return itr;
			}

			return end;
		}( );

		(void)first_valid;
		(void)last_valid;

		if (last_valid == end)
			ticks_window = std::span(first_valid, 1);
		else
			ticks_window = std::span(first_valid, last_valid);
	};

	(void)this;

	erase_uselles( );
	update_window( );
}

size_t player::max_ticks_count( )
{
	return utils::time_to_ticks(utils::unlag_limit( ) + utils::unlag_range( ));
}
