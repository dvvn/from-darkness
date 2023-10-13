#pragma once

#include <cstdint>
#include <utility>

namespace fd
{
enum class entity_index_source : uint8_t
{
    own,
    csgo,
    native = csgo
};

template <entity_index_source Source>
struct basic_entity_index;

template <typename T>
concept is_entity_index = requires(T obj) {
    []<entity_index_source Source>(basic_entity_index<Source>) {
    }(obj);
};

template <typename T = size_t>
constexpr T convert_entity_index(entity_index_source from, entity_index_source to, T index)
{
    if (from == to)
        return index;
    if (from == entity_index_source::native && to == entity_index_source::own)
        return index - 1;
    if (from == entity_index_source::own && to == entity_index_source::native)
        return index + 1;

    std::unreachable();
}

template <is_entity_index T>
constexpr auto convert_entity_index(T from, entity_index_source to) -> typename T::size_type
{
    return convert_entity_index<typename T::size_type>(from.source(), to, from);
}

// template <entity_index_source To, is_entity_index T>
// constexpr auto convert_entity_index(T index)
//{
//     return []<template <entity_index_source> class S>(S<T::source()> from) -> S<To>
//     {
//         return from;
//     }(index);
// }

template <entity_index_source Source>
struct basic_entity_index
{
    template <entity_index_source>
    friend struct basic_entity_index;

    using size_type = uint16_t;

  private:
    size_type index_;

  public:
    /*explicit*/ constexpr basic_entity_index(size_type index)
        : index_(convert_entity_index(entity_index_source::native, Source, index))
    {
    }

    template <entity_index_source OtherSource>
    constexpr basic_entity_index(basic_entity_index<OtherSource> other) requires(OtherSource != Source)
        : index_(convert_entity_index(other, Source))
    {
    }

    static constexpr entity_index_source source()
    {
        return Source;
    }

    constexpr operator size_type() const
    {
        return index_;
    }

    basic_entity_index &operator++()
    {
        ++index_;
        return *this;
    }
};

using native_entity_index = basic_entity_index<entity_index_source::native>;
using entity_index        = basic_entity_index<entity_index_source::own>;

} // namespace fd