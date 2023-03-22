#pragma once

#include <fd/valve/client_side/player.h>

#if __has_include(<fd/netvars_generated/C_CSPlayer_h_inc>)
#include <fd/netvars_generated/C_CSPlayer_h_inc>
#endif

namespace fd::valve::client_side
{
struct cs_player : player
{
#if __has_include(<fd/netvars_generated/C_CSPlayer_h>)
#include <fd/netvars_generated/C_CSPlayer_h>
#endif
    animating *rag_doll();
};
} // namespace fd::valve::client_side