#pragma once

#include "entity_cache/cached_player.h"

namespace fd
{
// for begin..end access, items not null
struct basic_cached_players_range
{
    using iterator = cached_player const *const *;

    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
} // namespace fd