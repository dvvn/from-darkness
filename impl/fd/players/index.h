#pragma once

#include <cstdint>
#include <utility>

namespace fd
{
enum class index_source : uint8_t
{
    own,
    csgo,
    game = csgo
};

template <index_source Source>
struct entity_index;

template <typename T>
concept is_entity_index = requires(T obj) {
    []<index_source Source>(entity_index<Source>) {
    }(obj);
};

template <typename T = size_t>
constexpr T convert_entity_index(index_source from, index_source to, T index)
{
    if (from == to)
        return index;
    if (from == index_source::game && to == index_source::own)
        return index - 1;
    if (from == index_source::own && to == index_source::game)
        return index + 1;
    
    std::unreachable();
}

template <is_entity_index T>
constexpr auto convert_entity_index(T from, index_source to) -> typename T::size_type
{
    return convert_entity_index<typename T::size_type>(from.source(), to, from);
}

// template <index_source To, is_entity_index T>
// constexpr auto convert_entity_index(T index)
//{
//     return []<template <index_source> class S>(S<T::source()> from) -> S<To>
//     {
//         return from;
//     }(index);
// }

template <index_source Source>
struct entity_index
{
    template <index_source>
    friend struct entity_index;

    using size_type = uint16_t;

  private:
    size_type index_;

  public:
    /*explicit*/ constexpr entity_index(size_type index)
        : index_(convert_entity_index(index_source::game, Source, index))
    {
    }

    template <index_source OtherSource>
    constexpr entity_index(entity_index<OtherSource> other)
        : index_(convert_entity_index(other, Source))
    {
    }

    static constexpr index_source source()
    {
        return Source;
    }

    constexpr operator size_type() const
    {
        return index_;
    }
};

static constexpr uint8_t max_player_count = 64;

constexpr bool validate_player_index(entity_index<index_source::csgo> index)
{
    return index != 0 && index <= max_player_count;
}

constexpr bool validate_player_index(entity_index<index_source::own> index)
{
    return index < max_player_count;
}

template <index_source Source>
struct player_index : entity_index<Source>
{
    using size_type = typename entity_index<Source>::size_type;

    constexpr player_index(size_type index)
        : entity_index<Source>((index))
    {
#ifdef _DEBUG
        if (!validate_player_index(*this))
            std::unreachable();
#endif
    }

    template <index_source OtherSource>
    constexpr player_index(entity_index<OtherSource> index)
        : entity_index<Source>(index)
    {
#ifdef _DEBUG
        if (!validate_player_index(index))
            std::unreachable();
#endif
    }

    template <index_source OtherSource>
    constexpr player_index(player_index<OtherSource> index)
        : entity_index<Source>(index)
    {
    }
};

template <index_source Source>
constexpr bool validate_player_index(player_index<Source> index, bool force = false)
{
    return !force || validate_player_index(static_cast<entity_index<Source>>(index));
}

} // namespace fd