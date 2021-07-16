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

void players_list::Load( )
{
	(void)this;
}

string players_list::Get_loaded_message( ) const
{
#ifndef CHEAT_GUI_TEST
	return service_base::Get_loaded_message( );
		//return "Players list ready to use";
#else
	return Get_loaded_message_disabled( );
#endif
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

	C_CSPlayer* const local_player = interfaces.local_player;
	const auto local_player_index = local_player->EntIndex( );
	const auto local_player_team = static_cast<m_iTeamNum_t>(local_player->m_iTeamNum( ));
	const auto local_player_alive = local_player->m_iHealth( ) > 0;

	for (auto i = 1; i <= max_clients; ++i)
	{
		const auto ent = i == local_player_index
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

		if (obj == nullptr || obj->ent != ent /*|| obj->index( ) != i*/)
		{
			storage_updated = true;
			player_shared new_obj;
			new_obj.init(ent);
			obj = move(new_obj);
		}

		const auto ent_team = static_cast<m_iTeamNum_t>(ent->m_iTeamNum( ));
		/*if (obj->team != ent_team)
		{
			storage_updated = true;
			obj->team = ent_team;
		}*/

		bool update_animations;
		bool store_tick;

		if (ent->m_iHealth( ) > 0)
		{
			if (!obj->alive)
			{
				obj->alive = true;
				storage_updated = true;
			}

			update_animations = true;
			store_tick = true;
		}
		else
		{
			if (obj->alive)
			{
				obj->alive = false;
				storage_updated = true;
			}

			const auto is_ragdoll_active = [&]
			{
				const auto ragdoll = ent->GetRagdoll( );
				return ragdoll != nullptr && ragdoll->m_nSequence( ) != -1 && !ragdoll->IsDormant( );
			};

			update_animations = is_ragdoll_active( );
			store_tick = false;
		}

		if (ent->IsDormant( ))
		{
			if (obj->dormant == false)
			{
				obj->dormant = true;
				storage_updated = true;
			}
			update_animations = false;
			store_tick = false;
		}
		else
		{
			if (obj->dormant == true)
			{
				obj->dormant = false;
				store_tick = false;
				storage_updated = true;
			}
		}

		if (!obj.update_simtime( ))
		{
			update_animations = false;
			store_tick = false;
		}

		if (!local_player_alive || local_player_team == ent_team || local_player_team == TEAM_Spectator)
			store_tick = false;

		if (store_tick)
			obj.store_tick( );
		if (update_animations)
			obj.update_animations(store_tick == false);

		obj.remove_old_ticks( );
	}

	if (storage_updated)
		filter_cache__.clear( );
#endif
}

const players_filter& players_list::filter(const players_filter_flags& flags)
{
	static_assert(sizeof(players_list_container_interface) == sizeof(players_list_container));
	return *filter_cache__.emplace(reinterpret_cast<const players_list_container_interface&>(storage__), flags).first;
}
