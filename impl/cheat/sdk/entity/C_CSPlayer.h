#pragma once
#include "C_BasePlayer.h"
#include "C_WeaponCSBase.h"

#include "cheat/sdk/IGameEventmanager.hpp"

namespace cheat::csgo
{
	////econ
	//class CAttributeManager;
	//class CAttributeList;
	//class CAttributeContainer;

	class C_CSPlayer: public C_BasePlayer
	{
	public:
#include "../generated/C_CSPlayer_h"
	};
}
