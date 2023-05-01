#pragma once

#include "basic_array.h"

namespace fd
{
struct player_array final : basic_player_array
{
    void update(player_index<index_source::own> index, player *value)
    {
        storage_[index] = value;
    }
};
}