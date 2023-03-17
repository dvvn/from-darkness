#pragma once

#include <fd/valve/cs_player.h>

namespace fd
{
class player
{
    valve::cs_player *ptr_;

    friend class players_list_global;

  public:
    player(valve::client_entity *ptr);

    bool valid() const;

    valve::cs_player *operator->() const;
    bool operator==(valve::client_entity const *ent) const;
    bool operator==(player const &other) const;
};

}