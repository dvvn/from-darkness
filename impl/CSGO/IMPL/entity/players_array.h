#pragma once

#include "player.h"
#include "player_index.h"

namespace fd
{
// for [] access, entries maybe null
struct basic_players_array
{
    using reference = player const &;

  protected:
    ~basic_players_array() = default;

  public:
    
    virtual reference operator[](own_player_index index) const = 0;
};
} // namespace fd