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
    //todo : pass fn by reference if not trivially copyable]
    iterate(begin(container), end(container), fn);
}
#if 0
template<typename Src, typename Dst>
concept can_memmove = std::is_trivialy_destructible_v<> && std::convertible_to<Src, const void*> && std::convertible_to<Dst, void*>;

template<typename Src, typename Dst>
constexpr void copy(Src begin, Src end, Dst dst)
{
    if constexpr(can_memmove<Src, Dst>)
    {
        if(!std::is_constant_evaluated())
        {
#ifdef _DEBUG
            if(begin == end)
                return;
#endif
            memmove(dst, begin, std::distance(begin, end) * sizeof(std::iter_value_t<Src>));
            return;
        }
    }

    while(begin != end)
    {
        *dst++ = *begin++;
    }
}
#endif

template<typename Src, typename Dst>
constexpr void copy(Src begin, size_t count, Dst dst)
{
    //todo: pass iterator by ref if not trivially copyable
    return copy(begin, begin + size, dst);
}

template<native_iterable T,typename Dst>
void copy(T&& container, Dst dst)
{
    copy(begin(container), end(container), dst);
}
}
