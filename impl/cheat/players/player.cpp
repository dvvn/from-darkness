#include "players list.h"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

player::player([[maybe_unused]] C_CSPlayer* ent)
{
#ifndef CHEAT_NETVARS_UPDATING

	this->sim_time = /*ent->m_flSimulationTime( )*/-1;
	this->dormant = ent->IsDormant( );
	this->in_use = true;
	//this->index = ent->EntIndex( );
	this->ent = ent;

#endif
}


