#pragma once
#include "C_BasePlayer.h"
#include "C_WeaponCSBase.h"

#include "cheat/sdk/IGameEventmanager.hpp"

namespace cheat::csgo
{
	// This abstracts the differences between CS players and hostages.
	class ICSPlayerAnimStateHelpers
	{
	public:
		
		virtual auto CSAnim_GetActiveWeapon( ) -> C_WeaponCSBase* = 0;
		virtual auto CSAnim_CanMove( ) -> bool = 0;
	};

	//econ
	class CAttributeManager;
	class CAttributeList;
	class CAttributeContainer;
	class IHasAttributes
	{
	public:
		virtual auto GetAttributeManager( ) -> CAttributeManager* = 0;
		virtual auto GetAttributeContainer( ) -> CAttributeContainer* = 0;
		virtual auto GetAttributeOwner( ) -> C_BaseEntity* = 0;
		virtual auto GetAttributeList( ) -> CAttributeList* = 0;

		// Reapply yourself to whoever you should be providing attributes to.
		virtual auto ReapplyProvision( ) -> void = 0;
	};

	class C_CSPlayer: public C_BasePlayer,
					  public /*CGameEventListener*/IGameEventListener2,
					  public ICSPlayerAnimStateHelpers
#if !defined( NO_STEAM ) && !defined( NO_STEAM_GAMECOORDINATOR )
					, public IHasAttributes
#endif
	{
	public:
	};
}
