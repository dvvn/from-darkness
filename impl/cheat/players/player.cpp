#include "players list.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

player::player([[maybe_unused]] C_CSPlayer* ent)
{
#ifndef CHEAT_NETVARS_UPDATING
	this->in_use__ = true;
	this->index__ = ent->EntIndex( );
	this->ent__ = ent;
	this->sim_time__ = ent->m_flSimulationTime( );
#endif
}

bool player::update_simtime( )
{
	//todo: tickbase shift workaround

#ifdef CHEAT_NETVARS_UPDATING
	(void)this;
	return false;
#else
	const auto new_sim_time = ent__->m_flSimulationTime( );

	if (sim_time__ >= new_sim_time)
		return false;
	sim_time__ = new_sim_time;
	return true;
#endif
}

void player::update_animations( )
{
	(void)this;
#ifndef CHEAT_NETVARS_UPDATING
	BOOST_ASSERT(ent__->m_flSimulationTime( )==sim_time__);
	//todo: proper animfix
	//or hook update_clientside_animation
	ent__->m_bClientSideAnimation( ) = true;
	ent__->UpdateClientSideAnimation( );
	ent__->m_bClientSideAnimation( ) = false;
#endif
}

C_CSPlayer* player::owner( ) const
{
	return ent__;
}

int player::index( ) const
{
	return index__;
}

C_CSPlayer* player::operator->( ) const
{
	return ent__;
}
