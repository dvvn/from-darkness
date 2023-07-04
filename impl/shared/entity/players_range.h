#pragma once

#include "player.h"

namespace fd
{
// for begin..end access, items not null
struct basic_players_range
{
    using iterator = player const *const *;

  protected:
    ~basic_players_range() = default;

  public:
    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};
} // namespace fd