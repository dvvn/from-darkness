#pragma once

#include "entity_index.h"

#include <cassert>

namespace fd
{
inline constexpr uint8_t max_player_count = 64;

constexpr bool validate_player_index(basic_entity_index<entity_index_source::csgo> index)
{
    return index != 0 && index <= max_player_count;
}

constexpr bool validate_player_index(basic_entity_index<entity_index_source::own> index)
{
    return index < max_player_count;
}

template <entity_index_source Source>
struct player_index : basic_entity_index<Source>
{
    using size_type = typename basic_entity_index<Source>::size_type;

    constexpr player_index(size_type index)
        : basic_entity_index<Source>((index))
    {
        assert(validate_player_index(*this));
    }

    template <entity_index_source OtherSource>
    constexpr player_index(basic_entity_index<OtherSource> index)
        : basic_entity_index<Source>(index)
    {
        assert(validate_player_index(index));
    }

    template <entity_index_source OtherSource>
    constexpr player_index(player_index<OtherSource> index) requires(OtherSource != Source)
        : basic_entity_index<Source>(index)
    {
    }
};

template <entity_index_source Source>
constexpr bool validate_player_index(player_index<Source> index) = delete;

using game_player_index = player_index<entity_index_source::native>;
using own_player_index  = player_index<entity_index_source::own>;
}