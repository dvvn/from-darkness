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
		virtual C_WeaponCSBase* CSAnim_GetActiveWeapon( ) = 0;
		virtual bool            CSAnim_CanMove( ) = 0;
	};

	//econ
	class CAttributeManager;
	class CAttributeList;
	class CAttributeContainer;
	class IHasAttributes
	{
	public:
		virtual CAttributeManager*   GetAttributeManager( ) = 0;
		virtual CAttributeContainer* GetAttributeContainer( ) = 0;
		virtual C_BaseEntity*        GetAttributeOwner( ) = 0;
		virtual CAttributeList*      GetAttributeList( ) = 0;

		// Reapply yourself to whoever you should be providing attributes to.
		virtual void ReapplyProvision( ) = 0;
	};

	class C_CSPlayer: public C_BasePlayer,
					  public /*CGameEventListener*/IGameEventListener2,
					  public ICSPlayerAnimStateHelpers
#if !defined( NO_STEAM ) && !defined( NO_STEAM_GAMECOORDINATOR )
					, public IHasAttributes
#endif
	{
	public:
#include "../generated/C_CSPlayer_h"
	};
}
