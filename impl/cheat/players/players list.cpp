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
	auto ent = holder.ent;
	this->origin = invoke(&C_BaseEntity::m_vecOrigin, ent);
	this->abs_origin = ent->m_vecAbsOrigin( );
	this->rotation = ent->m_angRotation( );
	this->abs_rotation = ent->m_angAbsRotation( );
	this->mins = ent->m_vecMins( );
	this->maxs = ent->m_vecMaxs( );
	this->sim_time = holder.sim_time;
	this->coordinate_frame = reinterpret_cast<matrix3x4_t&>(ent->m_rgflCoordinateFrame( ));
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

		//-------------

		if (obj == nullptr || obj->ent != ent /*|| obj->index( ) != i*/)
		{
			storage_updated = true;
			player_shared new_obj;
			new_obj.init(ent);
			obj = move(new_obj);
		}

		bool update_animations;
		bool store_tick;

		if (ent->m_iHealth( ) > 0)
		{
			update_animations = true;
			store_tick = true;
		}
		else
		{
			const auto is_ragdoll_active = [&]
			{
				const auto ragdoll = static_cast<C_BaseAnimating*>(interfaces.entity_list->GetClientEntityFromHandle(ent->m_hRagdoll( )));
				return ragdoll != nullptr && ragdoll->m_nSequence( ) != -1 && !ragdoll->IsDormant( );
			};

			update_animations = is_ragdoll_active( );
			store_tick = false;
		}
		if (ent->IsDormant( ))
		{
			update_animations = false;
			store_tick = false;
			obj->dormant = true;
		}
		else
		{
			if (obj->dormant)
				store_tick = false;
			obj->dormant = false;
		}
		if (!obj.update_simtime( ))
		{
			update_animations = false;
			store_tick = false;
		}

		//-------------

		if (update_animations)
			obj.update_animations( );

		if (store_tick)
			obj.store_tick( );

		obj.remove_old_ticks( );
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
