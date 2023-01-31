#pragma once

#include <fd/views.h>

namespace fd
{
template <typename T, size_t MaxSize = sizeof(void*) * 2>
using try_add_ref_t = std::conditional_t<!std::is_trivially_copyable_v<T> || (sizeof(T) > MaxSize), std::add_lvalue_reference_t<T>, T>;

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
    for (decltype(auto) v : container)
    {
        fn(static_cast<decltype(v)>(v));
    }
}

template <typename T, typename T2>
concept same_iter_size = sizeof(std::iter_value_t<T>) == sizeof(std::iter_value_t<T2>);

template <typename Src, typename Dst>
concept can_memmove = same_iter_size<Src, Dst> && /**/
                      requires(Src src, Dst dst) {
                          memmove(dst, src, 2u);
                          //*dst = *src;
                      };

template <typename Src, typename Dst>
constexpr void copy(Src begin, Src end, Dst dst)
{
    if constexpr (can_memmove<Src, Dst>)
    {
        if (!std::is_constant_evaluated())
        {
            memmove(dst, begin, std::distance(begin, end) * sizeof(std::iter_value_t<Src>));
            return;
        }
    }

    while (begin != end)
        *dst++ = *begin++;
}

template <typename Src, typename Dst>
constexpr void copy(Src src, size_t count, Dst dst)
{
    if constexpr (can_memmove<Src, Dst>)
    {
        if (!std::is_constant_evaluated())
        {
            memmove(dst, src, count * sizeof(Dst));
            return;
        }
    }

    while (--count != static_cast<size_t>(-1))
        *dst++ = *src++;
}

template <native_iterable T, typename Dst>
constexpr void copy(T&& container, Dst dst)
{
    auto tmp = forward_view_lazy(container);
    copy<std::remove_cvref_t<decltype(tmp.begin())>, try_add_ref_t<Dst>>(tmp.begin(), tmp.end(), dst);
}

template <typename T, typename T2>
concept can_memcmp = same_iter_size<T, T2> && /**/
                     requires(T left, T2 right) {
                         memcmp(left, right, 1u);
                         //*left == *right;
                     };

template <typename It, typename It2>
constexpr bool equal(It begin, It end, It2 with)
{
    if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
        {
            return memcmp(begin, with, std::distance(begin, end) + sizeof(std::iter_value_t<It>)) == 0;
        }
    }

    while (begin != end)
    {
        if (*begin++ != *with++)
            return false;
    }

    return true;
}

template <typename It, typename It2>
constexpr bool equal(It begin, size_t size, It2 with)
{
    if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
            return memcmp(begin, with, size + sizeof(std::iter_value_t<It>)) == 0;
    }

    while (--size != static_cast<size_t>(-1))
    {
        if (*begin++ != *with++)
            return false;
    }

    return true;
}

template <native_iterable T, typename It>
constexpr bool equal(T&& container, It with)
{
    auto tmp = forward_view_lazy(container);
    return equal<decltype(tmp.begin()), try_add_ref_t<It>>(tmp.begin(), tmp.end(), with);
}

template <typename T, typename T2>
concept can_memset = sizeof(std::iter_value_t<T>) == sizeof(T2) && std::floating_point<T2> == false && /**/
                     requires(T left, T2 val) {
                         memset(left, val, 2u);
                         //*left = val;
                     };

template <typename It, typename T>
constexpr void fill(It begin, It end, const T& val)
{
    if constexpr (can_memset<It, const T&>)
    {
        if (!std::is_constant_evaluated())
        {
            memset(begin, val, std::distance(begin, end) * sizeof(T));
            return;
        }
    }

    while (begin != end)
        *begin++ = val;
}

template <typename It, typename T>
constexpr void fill(It begin, size_t count, const T& val)
{
    if constexpr (can_memset<It, const T&>)
    {
        if (!std::is_constant_evaluated())
        {
            memset(begin, val, count * sizeof(T));
            return;
        }
    }

    while (--count != static_cast<size_t>(-1))
        *begin++ = val;
}

template <native_iterable T, typename T1>
constexpr void fill(T& container, const T1& val)
{
    auto tmp = forward_view_lazy(container);
    fill(tmp.begin(), tmp.end(), val);
}

// if constexpr(sizeof(std::iter_value_t<It>)==sizeof(char))

}