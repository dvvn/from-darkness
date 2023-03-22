#pragma once

#include <fd/valve/base_combat_character.h>

#if __has_include(<fd/netvars_generated/C_BasePlayer_h_inc>)
#include <fd/netvars_generated/C_BasePlayer_h_inc>
#endif

namespace fd::valve
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
};

struct base_player : base_combat_character
{
#if __has_include(<fd/netvars_generated/C_BasePlayer_h>)
#include <fd/netvars_generated/C_BasePlayer_h>
#endif
};
} // namespace fd::valve