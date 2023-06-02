#pragma once

#include "player.h"

namespace fd
{
// for begin..end access, items not null
struct basic_player_range
{
    using iterator = player *const *;

    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
} // namespace fd