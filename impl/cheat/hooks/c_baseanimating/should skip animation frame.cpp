#include "should skip animation frame.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/netvars/netvars.h"
#include "cheat/players/players list.h"
#include "cheat/sdk/ClientClass.hpp"
#include "cheat/sdk/IClientEntityList.hpp"

using namespace cheat;
using namespace hooks;
using namespace c_base_animating;

using namespace csgo;

should_skip_animation_frame::should_skip_animation_frame( )
{
}

bool should_skip_animation_frame::load_impl( )
{
	this->hook( );
	this->enable( );

	return true;
}

nstd::address should_skip_animation_frame::get_target_method_impl( ) const
{
	return find_signature("client.dll", "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02");
}

void should_skip_animation_frame::callback(/*float current_time*/)
{
	if (override_return__)
		this->return_value_.store_value(override_return_to__);
	else
	{
		constexpr auto is_player = [](IClientNetworkable* ent)
		{
			const auto client_class = ent->GetClientClass( );
			return client_class->ClassID == ClassId::CCSPlayer;
		};

		/*constexpr auto is_weapon = [](IClientNetworkable* ent)
		{
			const auto client_class = ent->GetClientClass( );
			const string_view name = client_class->pNetworkName;
			return name.find("Weapon") != name.npos;
		};*/

		C_BaseAnimating* ent;

		if (const auto inst = this->object_instance; is_player(inst))
		{
			ent = inst;
		}
		else
		{
#if 0
			//unreachable

			CBaseHandle owner_handle;

			if (is_weapon(inst))
			{
				auto wpn = (C_BaseCombatWeapon*)inst;
				owner_handle = wpn->m_hOwner( );
			}
			else
			{
				owner_handle = inst->m_hOwnerEntity( );
			}

			if (!owner_handle.IsValid( ))
				return;

			const auto owner = static_cast<C_CSPlayer*>(owner_handle.Get( ));

			if (!owner)
				return;

			if (!is_player(owner))
				return;
			if (csgo_interfaces::get_shared( )->local_player.get( ) == owner)
				return;

			ent = owner;
#endif

			return;
		}

		const auto animate_this_frame = ent->m_bClientSideAnimation( );
		const auto skip_this_frame    = animate_this_frame == false;
		this->return_value_.store_value(skip_this_frame);
	}
}

void should_skip_animation_frame::render( )
{
	ImGui::Checkbox("override return", &override_return__);
	if (override_return__)
	{
		const auto pop = nstd::memory_backup(ImGui::GetStyle( ).ItemSpacing.x, 0);
		(void)pop;

		ImGui::SameLine( );
		ImGui::Text(" to ");
		ImGui::SameLine( );
		if (ImGui::RadioButton("false ", override_return_to__ == false))
			override_return_to__ = false;
		ImGui::SameLine( );
		if (ImGui::RadioButton("true", override_return_to__ == true))
			override_return_to__ = true;
	}
}
