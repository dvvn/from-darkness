module;

#include <cstdint>
#include <limits>
#include <type_traits>

export module fd.valve.base_player;
export import fd.valve.base_combat_character;

using namespace fd::valve;

struct base_player : base_combat_character
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
            // not from game, sets manually in fd's players list
            ,
        BROKEN = std::numeric_limits<std::underlying_type_t<m_lifeState_t>>::max()
    };

#if __has_include("C_BasePlayer_generated_h")
#include "C_BasePlayer_generated_h"
#endif
};

export namespace fd::valve
{
    using ::base_player;
}
