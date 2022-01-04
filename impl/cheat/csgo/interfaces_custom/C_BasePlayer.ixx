module;

#include <cstdint>
#include <type_traits>
#include <limits>

export module cheat.csgo.interfaces:C_BasePlayer;
export import :C_BaseCombatCharacter;
#if __has_include("C_BasePlayer_generated.ixx")
export import : C_BasePlayer_generated;
#endif

export namespace cheat::csgo
{
	enum class m_lifeState_t :int32_t
	{
		// alive
		ALIVE = 0
		// playing death animation or still falling off of a ledge waiting to hit ground
		, DYING = 1
		// dead. lying still.
		, DEAD = 2
		//not from game, sets manually in cheat's players list
		, BROKEN = std::numeric_limits<std::underlying_type_t<m_lifeState_t>>::max( )
	};

	class C_BasePlayer :
		public C_BaseCombatCharacter
#if __has_include("C_BasePlayer_generated.ixx")
		, public C_BasePlayer_generated
#endif
	{
	};
}
