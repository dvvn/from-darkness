#pragma once

#include <fd/views.h>

#include <algorithm>

namespace fd
{
template <typename T, size_t MaxSize = sizeof(void*) * 2>
using try_add_ref_t = std::conditional_t<!std::is_lvalue_reference_v<T> && (!std::is_trivially_copyable_v<T> || sizeof(T) > MaxSize), std::add_lvalue_reference_t<T>, T>;

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
constexpr void copy(Src src, size_t rngCount, Dst dst)
{
    if constexpr (can_memmove<Src, Dst>)
    {
        if (!std::is_constant_evaluated())
        {
            memmove(dst, src, rngCount * sizeof(Dst));
            return;
        }
    }

    while (rngCount-- != 0)
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
            return memcmp(begin, with, std::distance(begin, end) * sizeof(std::iter_value_t<It>)) == 0;
    }

    while (begin != end)
    {
        if (*begin++ != *with++)
            return false;
    }

    return true;
}

template <typename It, typename It2>
constexpr bool equal(It begin, size_t rngCount, It2 with)
{
    if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
            return memcmp(begin, with, rngCount * sizeof(std::iter_value_t<It>)) == 0;
    }

    while (rngCount-- != 0)
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
constexpr void fill(It begin, size_t rngCount, const T& val)
{
    if constexpr (can_memset<It, const T&>)
    {
        if (!std::is_constant_evaluated())
        {
            memset(begin, val, rngCount * sizeof(T));
            return;
        }
    }

    while (rngCount-- != 0)
        *begin++ = val;
}

template <native_iterable T, typename T1>
constexpr void fill(T& container, const T1& val)
{
    auto tmp = forward_view_lazy(container);
    fill(tmp.begin(), tmp.end(), val);
}

template <typename It>
[[nodiscard]] static size_t _seek_to_ex(It& it, const void* it2)
{
    if constexpr (std::convertible_to<const void*, It>)
    {
        auto begin = it;
        it         = static_cast<It>(it2);
        return std::distance(begin, it);
    }
    else if constexpr (std::is_pointer_v<It>)
    {
        auto begin = it;
        it         = reinterpret_cast<It>(const_cast<void*>(it2));
        return std::distance(begin, it);
    }
    else
    {
        using ptr_t        = const uint8_t*;
        const void* tmp    = it;
        const auto  offset = std::distance(static_cast<ptr_t>(tmp), static_cast<ptr_t>(it2)) / sizeof(std::iter_value_t<It>);
        std::advance(it, offset);
        return offset;
    }
}

template <typename It>
static void _seek_to(It& it, const void* it2)
{
    if constexpr (std::convertible_to<const void*, It>)
    {
        it = static_cast<It>(it2);
    }
    else if constexpr (std::is_pointer_v<It>)
    {
        it = reinterpret_cast<It>(const_cast<void*>(it2));
    }
    else
    {
        using ptr_t        = const uint8_t*;
        const void* tmp    = it;
        const auto  offset = std::distance(static_cast<ptr_t>(tmp), static_cast<ptr_t>(it2)) / sizeof(std::iter_value_t<It>);
        std::advance(it, offset);
    }
}

// ReSharper disable once CppInconsistentNaming
void* _find_1(const void* rngStart, const void* rngEnd, uint8_t val);
// ReSharper disable once CppInconsistentNaming
void* _find_2(const void* rngStart, const void* rngEnd, uint16_t val);
// ReSharper disable once CppInconsistentNaming
void* _find_4(const void* rngStart, const void* rngEnd, uint32_t val);
// ReSharper disable once CppInconsistentNaming
void* _find_8(const void* rngStart, const void* rngEnd, uint64_t val);

template <typename It>
static It _cast_helper(void* ptr)
{
    return static_cast<std::iter_value_t<It>*>(ptr);
}

template <typename It, typename T>
static auto _find_trivial(It rngStart, It rngEnd, T val) requires(sizeof(std::iter_value_t<It>) == sizeof(T))
{
    if constexpr (sizeof(T) == sizeof(uint8_t))
        return _cast_helper<It>(_find_1(rngStart, rngEnd, val));
    else if constexpr (sizeof(T) == sizeof(uint16_t))
        return _cast_helper<It>(_find_2(rngStart, rngEnd, val));
    else if constexpr (sizeof(T) == sizeof(uint32_t))
        return _cast_helper<It>(_find_4(rngStart, rngEnd, val));
    else if constexpr (sizeof(T) == sizeof(uint64_t))
        return _cast_helper<It>(_find_8(rngStart, rngEnd, val));
}

template <typename It, typename T>
static It _find_trivial(It rngStart, It rngEnd, T val) requires(sizeof(std::iter_value_t<It>) > sizeof(T))
{
    return _find_trivial(rngStart, rngEnd, static_cast<std::iter_value_t<It>>(val));
}

template <typename It, typename T>
static It _find_trivial(It rngStart, It rngEnd, T val) requires(sizeof(std::iter_value_t<It>) < sizeof(T))
{
    const auto val1 = static_cast<std::iter_value_t<It>>(val);
    if (static_cast<T>(val1) != val)
    {
        for (; rngStart != rngEnd; ++rngStart)
        {
            if (*rngStart == val)
                return rngStart;
        }
        return rngEnd;
    }
    return _find_trivial(rngStart, rngEnd, val1);
}

template <typename It, typename T>
static bool _find_native(It& begin, size_t rngCount, const T& val)
{
    for (;;)
    {
        if (rngCount-- == 0)
            return false;
        if (*begin == val)
            return true;
        ++begin;
    }
}

template <typename It, typename T>
concept can_trivial_find = requires(It it, T val) { *_find_trivial(it, it + 1, val); };

template <typename It, typename T>
static constexpr bool _find_trivial_wrapped(It& rngBegin, size_t elementsCount, const T& val)
{
    auto end    = rngBegin + elementsCount;
    auto target = _find_trivial(rngBegin, end, val);

    const auto found = target != end;
    if (found)
        _seek_to(rngBegin, target);

    return found;
}

template <typename It, typename T>
static constexpr bool _find(It& rngBegin, size_t elementsCount, const T& val)
{
    if constexpr (can_trivial_find<It, T>)
    {
        if (!std::is_constant_evaluated())
            return _find_trivial_wrapped(rngBegin, elementsCount, val);
    }

    return _find_native(rngBegin, elementsCount, val);
}

template <typename It, typename T>
constexpr It find(It begin, It end, const T& val)
{
    if (_find(begin, std::distance(begin, end), val))
        return begin;
    return end;
}

template <typename It, typename T>
constexpr It find(It begin, size_t rngCount, const T& val)
{
    auto beginBackup = begin;
    if (_find(begin, rngCount, val))
        return begin;
    return beginBackup + rngCount;
}

template <typename It, typename It2>
static bool _search_native(It& begin, size_t srcSize, It2 targetBegin, const size_t targetCount)
{
    for (;;)
    {
        auto tempBegin       = begin;
        auto temptargetBegin = targetBegin;
        auto remaining       = targetCount;

        for (;;)
        {
            if (*tempBegin != *temptargetBegin)
            {
                srcSize -= _seek_to_ex(begin, tempBegin + 1);
                if (srcSize < targetCount)
                    return false;
                break;
            }
            --remaining;
            if (remaining == 0)
                return begin;
            ++tempBegin;
            ++temptargetBegin;
        }
    }
}

template <typename It, typename It2>
static bool _search_trivial(It& rngBegin, size_t rngCount, It2 targetBegin, const size_t targetCount)
{
    const auto& val  = *targetBegin;
    auto        last = rngBegin + rngCount;

    for (;;)
    {
        if (rngCount < targetCount)
            return false;

        auto found = _find_trivial<try_add_ref_t<It>>(rngBegin, last, val);

        if (!found)
            return false;

        if constexpr (can_memcmp<It, It2>)
        {
            if (memcmp(found + 1, targetBegin + 1, (targetCount - 1) * sizeof(std::iter_value_t<It2>)) == 0)
            {
                _seek_to(rngBegin, found);
                return true;
            }
        }
        else
        {
            It   itL   = found + 1;
            auto itR   = targetBegin + 1;
            auto count = targetCount - 1;
            while (*itL == *itR)
            {
                if (count-- == 0)
                {
                    _seek_to(rngBegin, found);
                    return true;
                }

                ++itL;
                ++itR;
            }
        }

        rngCount -= _seek_to_ex(rngBegin, found);
    }
}

template <typename It, typename It2>
static constexpr bool _search(It& srcBegin, size_t srcSize, It2 targetBegin, const size_t targetCount)
{
    if (srcSize < targetCount)
        return false;

    using it2_maybe_ref = try_add_ref_t<It2>;
    using it2_val       = std::iter_value_t<It2>;

    if constexpr (can_trivial_find<It, it2_val>)
    {
        if (!std::is_constant_evaluated())
            return _search_trivial<it2_maybe_ref>(srcBegin, srcSize, targetBegin, targetCount);
    }

    return _search_native<It, it2_maybe_ref>(srcBegin, srcSize, targetBegin, targetCount);
}

template <typename It, typename It2>
constexpr It find(It begin, const size_t srcSize, It2 targetBegin, const size_t targetCount)
{
    auto beginBackup = begin;
    if (!_search(begin, srcSize, targetBegin, targetCount))
        return beginBackup + srcSize;
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
        return beginBackup + srcSize;
    return begin;
}

template <typename It, typename It2>
constexpr It find(It begin, It end, It2 targetBegin, const size_t targetCount)
{
    if (!_search(begin, std::distance(begin, end), targetBegin, targetCount))
        return end;
    return begin;
}

template <native_iterable L, native_iterable R>
constexpr auto find(L&& left, R&& right) -> decltype(std::begin(left))
{
    auto rngL  = forward_view_lazy(left);
    auto rngR  = forward_view_lazy(right);
    auto begin = rngL.begin();
    if (!_search(begin, rngL.size(), rngR.begin(), rngR.size()))
        return rngL.end();
    return begin;
}

template <native_iterable L, typename It>
constexpr auto find(L&& left, It targetBegin, It targetEnd) -> decltype(std::begin(left))
{
    auto rngL  = forward_view_lazy(left);
    auto begin = rngL.begin();
    if (!_search(begin, rngL.size(), targetBegin, targetEnd))
        return rngL.end();
    return begin;
}

size_t test_algorithms();
}