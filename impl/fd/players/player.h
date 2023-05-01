#pragma once

#include <fd/valve/client_side/entity.h>

namespace fd
{
class player
{
    using entity = valve::client_side::entity;

    void *game_ptr_;

  public:
    player(entity *ptr);

    bool valid() const;
    void destroy();

    bool operator==(entity const &other) const;
    bool operator==(player const &other) const;
};

}