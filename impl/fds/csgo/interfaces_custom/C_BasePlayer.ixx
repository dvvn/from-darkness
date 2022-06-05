module;

#include <cstdint>
#include <limits>
#include <type_traits>

export module fds.csgo.interfaces.C_BasePlayer;
export import fds.csgo.interfaces.C_BaseCombatCharacter;

export namespace fds::csgo
{
    enum class m_lifeState_t : int32_t
    {
        // alive
        ALIVE = 0
            // playing death animation or still falling off of a ledge waiting to hit ground
            ,
        DYING = 1
            // dead. lying still.
            ,
        DEAD = 2
            // not from game, sets manually in fds's players list
            ,
        BROKEN = std::numeric_limits<std::underlying_type_t<m_lifeState_t>>::max()
    };

    class C_BasePlayer : public C_BaseCombatCharacter
    {
      public:
#if __has_include("C_BasePlayer_generated_h")
#include "C_BasePlayer_generated_h"
#endif
    };
} // namespace fds::csgo
