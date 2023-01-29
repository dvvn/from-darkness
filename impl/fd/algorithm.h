#pragma once

#include <fd/views.h>

namespace fd
{
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
    auto last = end(container);
    for (auto itr = begin(container); itr != last; ++itr)
    {
        fn(*itr);
    }
}
}