#pragma once

#include <fd/valve/client_side/cs_player.h>

namespace fd
{
class player
{
    using cs_player = valve::client_side::cs_player;
    using entity    = valve::client_side::entity;

    cs_player *ptr_;

    friend class players_list_global;

  public:
    player(entity *ptr);

    bool valid() const;

    cs_player *operator->() const;
    bool operator==(entity const *ent) const;
    bool operator==(player const &other) const;
};

}