#pragma once

#include "player.h"

#include <fd/core.h>

namespace fd
{
// for begin..end access, items not null
struct basic_player_range
{
    using iterator = _const<player **>;

    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
} // namespace fd