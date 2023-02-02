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
    auto tmp = forward_view_lazy(container);
    copy<std::remove_cvref_t<decltype(tmp.begin())>, try_add_ref_t<Fn>>(tmp.begin(), tmp.end(), fn);
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

template <typename T>
static bool _equal_native_cast(const void* left, const void* right)
{
    return *static_cast<const T*>(left) == *static_cast<const T*>(right);
}

template <typename T, typename... Args>
static bool _equal_native_cast(auto left, auto right, size_t offset)
{
    if (offset > 0)
    {
        if (!_equal_native_cast<T>((const uint8_t*)left + offset, (const uint8_t*)right + offset))
            return false;
    }
    else
    {
        if (!_equal_native_cast<T>(left, right))
            return false;
    }

    if constexpr (sizeof...(Args) == 0)
        return true;
    else
        return _equal_native_cast<Args...>(left, right, offset + sizeof(T));
}

template <typename... Args>
static constexpr size_t _size_of()
{
    return (sizeof(Args) + ...);
}

static bool _equal_memcmp_ex(const void* left, const void* right, const size_t bytesCount)
{
#define EQUAL_NATIVE_CAST(...)    \
    case _size_of<__VA_ARGS__>(): \
        return _equal_native_cast<__VA_ARGS__>(left, right __VA_OPT__(, 0));
    switch (bytesCount)
    {
#ifndef __RESHARPER__
        EQUAL_NATIVE_CAST(uint8_t);
        EQUAL_NATIVE_CAST(uint16_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint16_t);
        EQUAL_NATIVE_CAST(uint32_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint32_t);
        // EQUAL_NATIVE_CAST(uint16_t, uint32_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint16_t, uint32_t);
        EQUAL_NATIVE_CAST(uint64_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint16_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint16_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint32_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint8_t, uint32_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint16_t, uint32_t, uint64_t);
        // EQUAL_NATIVE_CAST(uint64_t, uint64_t);
#endif
    default:
        return memcmp(left, right, bytesCount) == 0;
    }
#undef EQUAL_NATIVE_CAST
}

static bool _equal_memcmp(const void* left, const void* right, const size_t bytesCount)
{
    return memcmp(left, right, bytesCount) == 0;
}

template <typename It, typename It2>
constexpr bool equal(It begin, It end, It2 with)
{
    if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
            return _equal_memcmp_ex(begin, with, std::distance(begin, end) * sizeof(std::iter_value_t<It>));
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
            return _equal_memcmp_ex(begin, with, size * sizeof(std::iter_value_t<It>));
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
concept can_memset = sizeof(std::iter_value_t<T>) == sizeof(T2) && /**/
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

template <typename It, typename T>
concept can_memchr = sizeof(std::iter_value_t<It>) == sizeof(T) && sizeof(T) == sizeof(char) && /**/
                     (!std::floating_point<T>) &&                                               /**/
                     requires(It buff, T val) {
                         memchr(buff, 2u, val);
                         //*left == *right;
                     };

template <typename It, typename It2>
static void _seek_to(It& it, It2* it2)
{
    if constexpr (std::convertible_to<It2*, It>)
        it = static_cast<It>(it2);
    else if constexpr (std::is_pointer_v<It>)
        it = (It)it2;
    else
    {
        using ptr_t     = const uint8_t*;
        const void* tmp = it;
        std::advance(it, std::distance(static_cast<ptr_t>(tmp), static_cast<ptr_t>(it2)) / sizeof(std::iter_value_t<It>));
    }
}

template <typename It>
static bool _find_memchr(It& begin, size_t rngSize, int val)
{
    auto found = memchr(begin, rngSize, val);
    if (found)
        _seek_to(begin, found);
    return found;
}

template <typename It, typename T>
constexpr It find(It begin, It end, const T& val)
{
    if constexpr (can_memchr<It, T>)
    {
        if (!std::is_constant_evaluated())
        {
            if (_find_memchr(begin, std::distance(begin, end), val))
                return begin;
            return end;
        }
    }
    /*else if constexpr (std::convertible_to<T, int> && !std::floating_point<T>)
    {
        if (!std::is_constant_evaluated() && val < std::numeric_limits<uint8_t>::max())
        {
            if (_find_memchr(begin, std::distance(begin, end), val))
                return begin;
            return end;
        }
    }*/

    for (; begin != end; ++begin)
    {
        if (*begin == val)
            return begin;
    }
    return end;
}

template <typename It, typename T>
constexpr It find(It begin, size_t size, const T& val)
{
    if constexpr (can_memchr<It, T>)
    {
        if (!std::is_constant_evaluated())
        {
            if (_find_memchr(begin, size, val))
                return begin;
            return begin + size;
        }
    }
    /*else if constexpr (std::convertible_to<T, int> && !std::floating_point<T>)
    {
        if (!std::is_constant_evaluated() && val < std::numeric_limits<uint8_t>::max())
        {
            if (_find_memchr(begin, size, val))
                return begin;
            return begin + size;
        }
    }*/

    while (size-- != (0) && *begin != val)
    {
        ++begin;
    }
    return begin;
}

template <typename It, typename It2>
static bool _equal_memcmp_t(It begin, It2 testBegin, const size_t testSize)
{
    return _equal_memcmp(begin, testBegin, testSize * sizeof(std::iter_value_t<It2>));
}

template <size_t TargetSize, typename... Args>
static bool _search_memchr_then_native_cast(auto& begin, size_t wholeRngSize, auto targetBegin)
{
    const int      target0     = *targetBegin;
    constexpr auto firstOffset = TargetSize - _size_of<Args...>();
    if constexpr (TargetSize == 1)
    {
        for (auto rngSize = wholeRngSize; rngSize <= wholeRngSize; rngSize -= TargetSize)
        {
            if (_find_memchr(begin, rngSize, target0))
                return true;
        }
    }
    else if constexpr (firstOffset == 0)
    {
        const auto target1It = targetBegin + 1;
        for (auto rngSize = wholeRngSize; rngSize <= wholeRngSize; rngSize -= TargetSize)
        {
            if (_find_memchr(begin, rngSize, target0))
            {
                if constexpr (sizeof...(Args) == 1)
                {
                    if (_equal_native_cast<Args...>(begin, target1It))
                        return true;
                }
                else
                {
                    if (_equal_native_cast<Args...>(begin, target1It, 0))
                        return true;
                }
                std::advance(begin, TargetSize - 1);
            }
        }
    }
    else
    {
        for (auto rngSize = wholeRngSize; rngSize <= wholeRngSize; rngSize -= TargetSize)
        {
            auto found = memchr(begin, rngSize, target0);
            if (found)
            {
                if (_equal_native_cast<Args...>(found, targetBegin, firstOffset))
                {
                    _seek_to(begin, found);
                    return true;
                }
                else
                {
                    _seek_to(begin, static_cast<const uint8_t*>(found) + TargetSize);
                }
            }
        }
    }

    return false;
}

template <typename It, typename It2>
static bool _search_memchr_then_memcmp(It& begin, size_t wholeRngSize, It2 targetBegin, size_t targetSize)
{
    //+-1 because we already found it by memchr
    const int  targetFront      = *targetBegin;
    const auto targetNext       = targetBegin + 1;
    const auto targetSizeMinus1 = targetSize - 1;

    for (auto rngSize = wholeRngSize; rngSize <= wholeRngSize; rngSize -= targetSize)
    {
        if (!_find_memchr(begin, targetFront, rngSize))
            return false;
        auto rngStart = begin;
        std::advance(begin, 1);
        if (_equal_memcmp_t(begin, targetNext, targetSizeMinus1))
        {
            begin = std::move(rngStart);
            return true;
        }
        std::advance(begin, targetSizeMinus1);
    }

    return false;
}

template <typename It, typename It2>
static bool _search_memcmp(It& begin, size_t wholeRngSize, It2 targetBegin, size_t targetSize)
{
    const auto step = targetSize % 2 == 0 ? targetSize : targetSize - 1;
    for (auto rngSize = wholeRngSize; rngSize <= wholeRngSize; rngSize -= step)
    {
        if (_equal_memcmp_t(begin, targetBegin, targetSize))
            return true;
        std::advance(begin, step);
    }
    return false;
}

template <typename It, same_iter_size<It> It2>
static constexpr bool _search(It& begin, const size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    if constexpr (can_memchr<It, std::iter_value_t<It2>>)
    {
        if (!std::is_constant_evaluated())
        {
#define SEARCH_MEMCHR_THEN_NATIVE_CAST(_N_, ...) \
    case _N_:                                    \
        return _search_memchr_then_native_cast<_N_, ##__VA_ARGS__>(begin, srcSize, targetBegin);

            switch (targetSize)
            {
                SEARCH_MEMCHR_THEN_NATIVE_CAST(1);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(2, uint16_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(3, uint16_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(4, uint32_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(5, uint32_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(8, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(9, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(16, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(17, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(24, uint64_t, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(25, uint64_t, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(32, uint64_t, uint64_t, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(33, uint64_t, uint64_t, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(64, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
                SEARCH_MEMCHR_THEN_NATIVE_CAST(65, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
            default: {
                return (_search_memchr_then_memcmp(begin, srcSize, targetBegin, targetSize));
            }
            }
#undef SEARCH_MEMCHR_THEN_NATIVE_CAST
            return false;
        }
    }
    else if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
            return _search_memcmp(begin, srcSize, targetBegin, targetSize);
    }

    for (auto rngSize = srcSize; rngSize <= srcSize; rngSize -= targetSize)
    {
        if (equal<It&>(begin, targetSize, targetBegin))
            return true;
        std::advance(begin, targetSize);
    }
    return false;
}

template <typename It, typename It2>
constexpr It find(It begin, const size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    auto beginBackup = begin;
    if (!_search(begin, srcSize, targetBegin, targetSize))
        std::advance(begin, srcSize - std::distance(beginBackup, begin));
    return begin;
}

template <typename It, typename It2>
constexpr It find(It begin, It end, It2 targetBegin, It2 targetEnd)
{
    if (!_search(begin, std::distance(begin, end), targetBegin, std::distance(targetBegin, targetEnd)))
        return end;
    return begin;
}

template <typename It, typename It2>
constexpr It find(It begin, const size_t srcSize, It2 targetBegin, It2 targetEnd)
{
    auto beginBackup = begin;
    if (!_search(begin, srcSize, targetBegin, std::distance(targetBegin, targetEnd)))
        std::advance(begin, srcSize - std::distance(beginBackup, begin));
    return begin;
}

template <typename It, typename It2>
constexpr It find(It begin, It end, It2 targetBegin, const size_t targetSize)
{
    if (!_search(begin, std::distance(begin, end), targetBegin, targetSize))
        return end;
    return begin;
}

template <native_iterable L, native_iterable R>
constexpr auto find(L&& left, R&& right) -> decltype(std::begin(left))
{
    auto l     = forward_view(left);
    auto r     = forward_view_lazy(right);
    auto begin = l.begin();
    if (!_search(begin, l.size(), r.begin(), r.size()))
        return l.end();
    return begin;
}

template <native_iterable L, typename It>
constexpr auto find(L&& left, It targetBegin, It targetEnd) -> decltype(std::begin(left))
{
    auto l     = forward_view(left);
    auto begin = l.begin();
    if (!_search(begin, l.size(), targetBegin, targetEnd))
        return l.end();
    return begin;
}
}