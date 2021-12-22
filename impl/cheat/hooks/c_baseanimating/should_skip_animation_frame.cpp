#include "should_skip_animation_frame.h"

#include "cheat/csgo/ClientClass.hpp"
#include "cheat/csgo/entity/C_BaseAnimating.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

void should_skip_animation_frame_impl::callback(/*float current_time*/)
{
	if (override_return__)
	{
		this->store_return_value(override_return_to__);
		return;
	}

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

	if (const auto inst = this->get_object_instance(); is_player(inst))
	{
		ent = inst;
	}
	else
	{
#if 0
			//unreachable

			CBaseHandle owner_handle;

			if(is_weapon(inst))
			{
				auto wpn = (C_BaseCombatWeapon*)inst;
				owner_handle = wpn->m_hOwner( );
			}
			else
			{
				owner_handle = inst->m_hOwnerEntity( );
			}

			if(!owner_handle.IsValid( ))
				return;

			const auto owner = static_cast<C_CSPlayer*>(owner_handle.Get( ));

			if(!owner)
				return;

			if(!is_player(owner))
				return;
			if(csgo_interfaces::get_shared( )->local_player.get( ) == owner)
				return;

			ent = owner;
#endif

		return;
	}

	const auto animate_this_frame = ent->m_bClientSideAnimation( );
	const auto skip_this_frame    = animate_this_frame == false;
	this->store_return_value(skip_this_frame);
}

#if 0
bool should_skip_animation_frame_impl::render( )
{
	ImGui::Checkbox("override return", &override_return__);
	if (!override_return__)
		return true;

	const auto pop = nstd::mem::backup(ImGui::GetStyle( ).ItemSpacing.x, 0);
	(void)pop;

	ImGui::SameLine( );
	ImGui::Text(" to ");
	ImGui::SameLine( );
	if (ImGui::RadioButton("false ", override_return_to__ == false))
		override_return_to__ = false;
	ImGui::SameLine( );
	if (ImGui::RadioButton("true", override_return_to__ == true))
		override_return_to__ = true;

	return true;
}
#endif