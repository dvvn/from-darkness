#pragma once

#include "entity_cache/cached_player.h"
#include "entity_cache/player_index.h"

namespace fd
{
// for [] access, entries maybe null
struct basic_cached_players_array
{
    using pointer = cached_player const *;

    virtual pointer operator[](own_player_index index) const = 0;
};
} // namespace fd