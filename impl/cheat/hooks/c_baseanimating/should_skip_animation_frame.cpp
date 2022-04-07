module;

#include <cheat/hooks/instance.h>

module cheat.hooks.c_base_animating.should_skip_animation_frame;
import cheat.csgo.modules;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.hooks.base;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(c_base_animating, should_skip_animation_frame);

static void* target( ) noexcept
{
	return csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">( );
}

struct replace
{
	void fn(/*float current_time*/) noexcept
	{
		CHEAT_HOOK_CALL_ORIGINAL_MEMBER( );

#if 0
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

		if (const auto inst = this->get_object_instance( ); is_player(inst))
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
		const auto skip_this_frame = animate_this_frame == false;
		this->store_return_value(skip_this_frame);
#endif
	}
};

CHEAT_HOOK_INIT(c_base_animating, should_skip_animation_frame);

