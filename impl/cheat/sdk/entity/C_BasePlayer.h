#pragma once
#include "C_BaseCombatCharacter.h"



namespace cheat::csgo
{
	enum LifeState : uint8_t
{
    LIFE_ALIVE = 0,// alive
    LIFE_DYING = 1, // playing death animation or still falling off of a ledge waiting to hit ground
    LIFE_DEAD = 2, // dead. lying still.
    MAX_LIFESTATE
};
	
	class C_BasePlayer: public C_BaseCombatCharacter
	{
	public:
#include "../generated/C_BasePlayer_h"
	};
}
