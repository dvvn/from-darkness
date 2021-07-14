#include "players list.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/gui/renderer.h"
#include "cheat/netvars/netvars.h"

#include "cheat/sdk/GlobalVars.hpp"
#include "cheat/sdk/IClientEntityList.hpp"
#include "cheat/sdk/IVEngineClient.hpp"

using namespace cheat;
using namespace detail;
using namespace csgo;
using namespace utl;

player::tick_record::tick_record([[maybe_unused]] player& holder)
{
#ifndef CHEAT_NETVARS_UPDATING
	this->origin__ = invoke(&C_BaseEntity::m_vecOrigin, holder.owner( ));
	this->abs_origin__ = holder->m_vecAbsOrigin( );
	this->rotation__ = holder->m_angRotation( );
	this->abs_rotation__ = holder->m_angAbsRotation( );
	this->mins__ = holder->m_vecMins( );
	this->maxs__ = holder->m_vecMaxs( );
	this->sim_time__ = holder.sim_time__;
	this->coordinate_frame__ = reinterpret_cast<matrix3x4_t&>(holder->m_rgflCoordinateFrame( ));
#endif
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
	const auto& interfaces = csgo_interfaces::get( );

	auto storage_updated = false;

	const auto max_clients = interfaces.global_vars->max_clients;
	if (const auto wished_storage_size = max_clients + 1;
		storage__.size( ) != wished_storage_size)
	{
		storage_updated = true;
		storage__.clear( );
		storage__.resize(wished_storage_size);
	}

	for (auto i = 1; i <= max_clients; ++i)
	{
		const auto ent = interfaces.local_player->EntIndex( ) == i
						 ? nullptr
						 : static_cast<C_CSPlayer*>(interfaces.entity_list->GetClientEntity(i));
		auto& obj = storage__[i];

		if (ent == nullptr)
		{
			if (obj != nullptr)
			{
				storage_updated = true;
				obj = { };
			}
			continue;
		}
		if (obj == nullptr)
		{
			storage_updated = true;
			obj.init(ent);
		}
		else if (obj->owner( ) != ent || obj->index( ) != i)
		{
			storage_updated = true;
			obj = { };
			obj.init(ent);
		}
		else
		{
			if (ent->m_iHealth( ) <= 0)
			{
				if (const auto ragdoll = static_cast<C_BaseAnimating*>(interfaces.entity_list->GetClientEntityFromHandle(ent->m_hRagdoll( )));
					ragdoll == nullptr || ragdoll->m_nSequence( ) == -1)
					continue;
			}

			if (ent->IsDormant( )) //todo: animations after player exit dormancy
				continue;

			if (!obj->update_simtime( ))
				continue;
		}
		obj->update_animations( );
	}

	if (storage_updated)
		filter_cache__.clear( );
#endif
}

const players_filter& players_list::filter(players_filter_bitflags flags)
{
	static_assert(sizeof(players_list_container_interface) == sizeof(players_list_container));
	return *filter_cache__.emplace(reinterpret_cast<const players_list_container_interface&>(storage__), flags).first;
}

void players_list::Load( )
{
	(void)this;
}

string players_list::Get_loaded_message( ) const
{
	return "Players list ready to use";
}
