#pragma once

#include "player.h"

namespace fd
{
// for begin..end access, items not null
struct basic_player_range
{
    using iterator = std::add_const_t<player *> *;

    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
}