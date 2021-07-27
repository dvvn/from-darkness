#include "players list.h"
#include "player.h"

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

players_list::players_list( )
{
}

bool players_list::Do_load( )
{
#ifdef CHEAT_GUI_TEST
	return 0;
#else

	return 1;
#endif


}



void players_list::update( )
{
#ifdef CHEAT_NETVARS_UPDATING
	(void)this;
#else
	const auto interfaces = csgo_interfaces::get_shared( );

	auto storage_updated = false;

	const auto max_clients = interfaces->global_vars->max_clients;
	if (const size_t wished_storage_size = max_clients + 1;
		storage__.size( ) != wished_storage_size)
	{
		storage_updated = true;
		storage__.clear( );
		storage__.resize(wished_storage_size);
	}

	C_CSPlayer* const local_player = interfaces->local_player;
	const m_iTeamNum_t local_player_team = local_player->m_iTeamNum( );
	const auto local_player_alive = local_player->IsAlive( );

	const auto ent_by_index = [local_player_index = local_player->EntIndex( ), &interfaces](int idx)
	{
		C_CSPlayer* ret;

		if (idx == local_player_index)
			ret = nullptr;
		else
			ret = static_cast<C_CSPlayer*>(interfaces->entity_list->GetClientEntity(idx));

		return ret;
	};

	const auto fixed_curtime = interfaces->global_vars->curtime; //todo

	for (auto i = 1; i <= max_clients; ++i)
	{
		const auto ent = ent_by_index(i);
		auto& obj = storage__[i];
		auto& obj_shared = obj.share( );

		if (ent == nullptr)
		{
			if (obj != nullptr)
			{
				storage_updated = true;
				obj = { };
			}
			continue;
		}

		if (obj == nullptr || obj_shared->ent != ent /*|| obj->index( ) != i*/)
		{
			storage_updated = true;
			players_list_container::value_type new_obj;
			new_obj.init(ent);
			obj = move(new_obj);
		}

		const m_iTeamNum_t ent_team = ent->m_iTeamNum( );
		if (obj_shared->team != ent_team)
		{
			storage_updated = true;
			obj_shared->team = ent_team;
		}

		bool update_animations;
		bool store_tick;

		if (ent->IsAlive( ))
		{
			if (!obj_shared->alive)
			{
				obj_shared->alive = true;
				storage_updated = true;
			}

			update_animations = true;
			store_tick = true;
		}
		else
		{
			if (obj_shared->alive)
			{
				obj_shared->alive = false;
				storage_updated = true;
			}

			const auto is_ragdoll_active = [ent]
			{
				const auto ragdoll = ent->GetRagdoll( );
				return ragdoll != nullptr && ragdoll->m_nSequence( ) != -1 && !ragdoll->IsDormant( );
			};

			update_animations = is_ragdoll_active( );
			store_tick = false;
		}

		if (ent->IsDormant( ))
		{
			if (obj_shared->dormant == false)
			{
				obj_shared->dormant = true;
				storage_updated = true;
			}
			update_animations = false;
			store_tick = false;
		}
		else
		{
			if (obj_shared->dormant == true)
			{
				obj_shared->dormant = false;
				store_tick = false;
				storage_updated = true;
			}
		}

		if (!obj.update_simtime( ))
		{
			update_animations = false;
			store_tick = false;
		}

		if (!local_player_alive || local_player_team == ent_team || local_player_team.spectator( ))
			store_tick = false;

		if (store_tick)
		{
			obj.store_tick( );
		}
		if (update_animations)
		{
			const auto backup = memory_backup(ent->m_bClientSideAnimation( ), true);
			(void)backup;

			obj.update_animations(store_tick == false);
			ent->SetupBones(nullptr, -1, BONE_USED_BY_ANYTHING, fixed_curtime);
			if (store_tick)
			{
				obj_shared->ticks.front( ).store_bones(ent);
				//store bones, animations, and do all useful shit
			}
		}

		obj.remove_old_ticks(fixed_curtime);
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
