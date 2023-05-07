#pragma once

#include "index.h"
#include "player.h"

namespace fd
{
// for [] access, entries maybe null
class basic_player_array
{
    friend struct player_array;

    player *storage_[max_player_count];

  public:
    template <index_source Source = index_source::own>
    player *operator[](player_index<Source> index) const
    {
#ifdef _DEBUG
        if constexpr (Source == index_source::own)
            return storage_[index];
        else
#endif
            return storage_[convert_entity_index(index, index_source::own)];
    }
};
} // namespace fd