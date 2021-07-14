#include "players list.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

void player_shared::init(C_CSPlayer* owner)
{
	BOOST_ASSERT(!*this);
	*static_cast<player::shared_type*>(this) = utl::make_shared<player>(owner);
}

player_shared::~player_shared( )
{
#ifndef CHEAT_NETVARS_UPDATING
	if (*this == nullptr)
		return;
	BOOST_ASSERT(get()->in_use);
	get( )->in_use = false;

	const auto reset_clientside_anim = [&]
	{
		__try
		{
			get( )->ent->m_bClientSideAnimation( ) = true;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	};
	reset_clientside_anim( );
#endif
}

player_shared::player_shared(player_shared&& other) noexcept
{
	*this = move(other);
}

void player_shared::operator=(player_shared&& other) noexcept
{
	*static_cast<player::shared_type*>(this) = move(other);
}

player::shared_type player_shared::share( ) const
{
	return *this;
}

bool player_shared::update_simtime( )
{
	//todo: tickbase shift workaround
	(void)this;
#ifdef CHEAT_NETVARS_UPDATING
	return false;
#else

	auto&      p = *this->get( );
	const auto new_sim_time = p.ent->m_flSimulationTime( );

	if (p.sim_time >= new_sim_time)
		return false;
	p.sim_time = new_sim_time;
	return true;
#endif
}

void player_shared::update_animations( )
{
	(void)this;
#ifndef CHEAT_NETVARS_UPDATING
	auto& p = *this->get( );
	(void)p;
	BOOST_ASSERT(p.ent->m_flSimulationTime( )==p.sim_time);
	//todo: proper animfix
	//
	//or hook update_clientside_animation
	p.ent->m_bClientSideAnimation( ) = true;
	p.ent->UpdateClientSideAnimation( );
	p.ent->m_bClientSideAnimation( ) = false;
#endif
}

void player_shared::store_tick( )
{
	(void)this;
	//todo
}

void player_shared::remove_old_ticks( )
{
	(void)this;
	//todo
}
