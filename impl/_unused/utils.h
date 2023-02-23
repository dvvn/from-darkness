#pragma once

#include <fd/class_info.h>

#include <boost/range/adaptors.hpp>

namespace fd
{
template <typename T>
constexpr void destroy_delete(T& rng)
{
    if constexpr (has_correct_destuction_order<T>())
    {
        for (auto* v : rng)
            delete v;
    }
    else
    {
        for (auto* v : boost::adaptors::reverse(rng))
            delete v;
    }

    rng.clear();
}

template <typename T>
constexpr void destroy(T& rng)
{
    if constexpr (has_correct_destuction_order<T>())
    {
        for (auto& v : rng)
            std::destroy_at(&v);
    }
    else
    {
        for (auto& v : boost::adaptors::reverse(rng))
            std::destroy_at(&v);
    }

    rng.clear();
}
}