#pragma once

#include <fd/valve/base_player.h>

#if __has_include(<fd/netvars_generated/C_CSPlayer_h_inc>)
#include <fd/netvars_generated/C_CSPlayer_h_inc>
#endif

namespace fd::valve
{
struct cs_player : base_player
{
#if __has_include(<fd/netvars_generated/C_CSPlayer_h>)
#include <fd/netvars_generated/C_CSPlayer_h>
#endif
    base_animating *rag_doll();
};
} // namespace fd::valve