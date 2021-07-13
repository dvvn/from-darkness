#include "players list.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/gui/renderer.h"
#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/IClientEntityList.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace csgo;
using namespace utl;

player::player([[maybe_unused]] C_CSPlayer* owner)
{
#ifndef CHEAT_NETVARS_UPDATING
	this->in_use__ = true;
	this->index__ = owner->EntIndex( );
	this->owner__ = owner;

	this->origin__ = invoke(&C_BaseEntity::m_vecOrigin, owner);
	this->abs_origin__ = owner->m_vecAbsOrigin( );
	this->rotation__ = owner->m_angRotation( );
	this->abs_rotation__ = owner->m_angAbsRotation( );
	this->mins__ = owner->m_vecMins( );
	this->maxs__ = owner->m_vecMaxs( );
	this->sim_time__ = owner->m_flSimulationTime( );
	this->coordinate_frame__ = reinterpret_cast<matrix3x4_t&>(owner->m_rgflCoordinateFrame( ));
#endif
}

bool player::update_simtime( )
{
	//todo: tickbase shift workaround

#ifdef CHEAT_NETVARS_UPDATING
	(void)this;
	return false;
#else
	const auto new_sim_time = owner__->m_flSimulationTime( );

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
	BOOST_ASSERT(owner__->m_flSimulationTime( )==sim_time__);
	//todo: proper animfix
	//or hook update_clientside_animation
	owner__->m_bClientSideAnimation( ) = true;
	owner__->UpdateClientSideAnimation( );
	owner__->m_bClientSideAnimation( ) = false;
#endif
}

C_CSPlayer* player::owner( ) const
{
	return owner__;
}

void player_shared_obj::init(C_CSPlayer* owner)
{
	BOOST_ASSERT(!pl__);
	pl__ = utl::make_shared<player>(owner);
}

player_shared_obj::~player_shared_obj( )
{
#ifndef CHEAT_NETVARS_UPDATING
	if (!pl__)
		return;
	BOOST_ASSERT(pl__->in_use__);
	pl__->in_use__ = false;

	const auto reset_clientside_anim = [&]
	{
		__try
		{
			pl__->owner__->m_bClientSideAnimation( ) = true;

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}

	};
	reset_clientside_anim( );
#endif
}

player_shared_obj::player_shared_obj(player_shared_obj&& other) noexcept
{
	*this = (move(other));
}

void player_shared_obj::operator=(player_shared_obj&& other) noexcept
{
	pl__ = move(other.pl__);
}

player_shared player_shared_obj::share( ) const
{
	return pl__;
}

player* player_shared_obj::operator->( ) const
{
	return pl__.get( );
}

bool player_shared_obj::operator!( ) const
{
	return !pl__;
}

players_list::players_list( )
{
	this->Wait_for<gui::renderer>( );
	this->Wait_for<netvars>( );
}

void players_list::update( )
{
#ifdef CHEAT_NETVARS_UPDATING
	(void)this;
#else
	const auto& ifc = csgo_interfaces::get( );

	for (auto i = 1; i <= ifc.global_vars->max_clients; ++i)
	{
		const auto ent = ifc.local_player->EntIndex( ) == i
						 ? nullptr
						 : static_cast<C_CSPlayer*>(ifc.entity_list->GetClientEntity(i));
		const auto ent_is_null = ent == nullptr;
		auto&      obj = storage__[i];

		const auto created = !obj || obj->owner( ) != ent;
		if (created)
		{
			player_shared_obj new_obj;
			if (!ent_is_null)
				new_obj.init(ent);
			obj = move(new_obj);
		}

		//-----

		if (ent_is_null)
			continue;
		if (ent->m_iHealth( ) <= 0)
			continue;
		if (ent->IsDormant( )) //todo: animations
			continue;

		if (!created && !obj->update_simtime( ))
			continue;

		obj->update_animations( );
	}
#endif
}

void players_list::Load( )
{
	(void)this;
}

string players_list::Get_loaded_message( ) const
{
	return "Players list ready to use";
}
