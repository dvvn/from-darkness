#pragma once
#include "C_BasePlayer.h"
#include "C_WeaponCSBase.h"

namespace cheat::csgo
{
	////econ
	//class CAttributeManager;
	//class CAttributeList;
	//class CAttributeContainer;

	class C_CSPlayer: public C_BasePlayer
	{
	public:
#if __has_include("../generated/C_CSPlayer_h")
#include "../generated/C_CSPlayer_h"
#endif

		C_BaseAnimating* GetRagdoll( );
	};
}
