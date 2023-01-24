#pragma once

#include <iterator>

namespace fd
{
template <typename T>
concept native_iterable = requires(T container) {
                              std::begin(container);
                              std::end(container);
                          };

template <typename It, typename Fn>
void iterate(It begin, It end, Fn fn)
{
    for (; begin != end; ++begin)
    {
        fn(*begin);
    }
}

template <native_iterable T, typename Fn>
void iterate(T&& container, Fn fn)
{
    auto end = std::end(container);
    for (auto itr = std::begin(container); itr != end; ++itr)
    {
        fn(*itr);
    }
}
}