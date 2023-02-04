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
constexpr void copy(Src src, size_t rngSize, Dst dst)
{
    if constexpr (can_memmove<Src, Dst>)
    {
        if (!std::is_constant_evaluated())
        {
            memmove(dst, src, rngSize * sizeof(Dst));
            return;
        }
    }

    while (rngSize-- != 0)
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
constexpr bool equal(It begin, size_t rngSize, It2 with)
{
    if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
            return memcmp(begin, with, rngSize * sizeof(std::iter_value_t<It>)) == 0;
    }

    while (rngSize-- != 0)
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
constexpr void fill(It begin, size_t rngSize, const T& val)
{
    if constexpr (can_memset<It, const T&>)
    {
        if (!std::is_constant_evaluated())
        {
            memset(begin, val, rngSize * sizeof(T));
            return;
        }
    }

    while (rngSize-- != 0)
        *begin++ = val;
}

template <native_iterable T, typename T1>
constexpr void fill(T& container, const T1& val)
{
    auto tmp = forward_view_lazy(container);
    fill(tmp.begin(), tmp.end(), val);
}

template <typename It, typename T>
concept can_memchr_barelly = sizeof(std::iter_value_t<It>) == sizeof(T) && /**/
                             requires(It buff, T val) {
                                 memchr(buff, val, 2);
                                 //*left == *right;
                             };

template <typename It, typename T>
concept can_memchr_convertible = std::convertible_to<T, std::iter_value_t<It>> && can_memchr_barelly<It, std::iter_value_t<It>>;

template <typename It, typename T>
concept can_memchr = sizeof(T) == sizeof(char) && (!std::floating_point<T>) && can_memchr_barelly<It, T>;

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

template <typename T>
static int _read_first_byte(const T& val)
{
    static_assert(sizeof(T) > sizeof(uint8_t));
    const auto firstByte = reinterpret_cast<const uint8_t&>(val);
    if (firstByte > std::numeric_limits<uint8_t>::max())
        return 0;
    return firstByte;
}

template <typename It, typename T>
static bool _find_native(It& begin, size_t rngSize, const T& val)
{
    for (;;)
    {
        if (rngSize-- == 0)
            return false;
        if (*begin == val)
            return true;
        ++begin;
    }
}

template <typename It, typename T>
static bool _find_memchr_barelly(It& rngBegin, size_t rngSize, const T& val)
{
    static_assert(sizeof(std::iter_value_t<It>) >= sizeof(T));

    const auto firstTargetByte = _read_first_byte(val);
    if (!firstTargetByte)
        return _find_native(rngBegin, rngSize, val);

    const auto absBegin = reinterpret_cast<uintptr_t>(static_cast<const void*>(rngBegin));

    for (;;)
    {
        auto found = reinterpret_cast<uintptr_t>(memchr(rngBegin, firstTargetByte, rngSize * sizeof(T)));
        if (!found)
            return false;

        const auto offset = (found - absBegin) % sizeof(T);
        if (offset != 0)
        {
            found += sizeof(T) - offset;
        }
        else if (*reinterpret_cast<std::iter_value_t<It>*>(found) == val)
        {
            _seek_to(rngBegin, reinterpret_cast<void*>(found));
            return true;
        }

        rngSize -= _seek_to_ex(rngBegin, reinterpret_cast<void*>(found));
        if (rngSize < sizeof(T))
            return false;
    }
}

template <typename It, typename T>
static bool _find_memchr(It& begin, size_t rngSize, const T& val)
{
    auto found = memchr(begin, val, rngSize);
    if (found)
        _seek_to(begin, found);
    return found;
}

template <typename It, typename T>
static bool _find_memchr_convert(It& begin, size_t rngSize, const T& val)
{
    using it_val = std::iter_value_t<It>;

    const auto itVal = static_cast<it_val>(val);
    if constexpr (sizeof(it_val) < sizeof(T))
    {
        if constexpr (can_memchr<It, it_val>)
        {
            if (static_cast<T>(itVal) == val)
                return _find_memchr(begin, rngSize, itVal);
        }
        return _find_native(begin, rngSize, itVal);
    }
    else
    {
        if constexpr (can_memchr<It, it_val>)
            return _find_memchr(begin, rngSize, itVal);
        else
            return _find_memchr_barelly(begin, rngSize, itVal);
    }
}

template <typename It, typename T>
concept can_prefer_std_algorithm =
#if !defined(_DEBUG) && defined(_MSC_VER) && defined(_USE_STD_VECTOR_ALGORITHMS)
    std::is_trivial_v<std::iter_value_t<It>> && std::is_trivial_v<T> && /**/
    sizeof(std::iter_value_t<It>) <= sizeof(uint64_t) && sizeof(T) <= sizeof(uint64_t);
#else
    false;
#endif

template <typename It, typename T>
static constexpr uint8_t _find_std(It& rngBegin, size_t rngSize, const T& val)
{
    std::iter_value_t<It> val1 = val;
    if constexpr (sizeof(std::iter_value_t<It>) < sizeof(T))
    {
        if (static_cast<T>(val1) != val)
            return 2;
    }

    auto last  = rngBegin + rngSize;
    It   found = __std_find_trivial<std::iter_value_t<It>>(rngBegin, last, val1);

    if (found != last)
    {
        _seek_to(rngBegin, found);
        return 1;
    }
    return 0;
}

template <typename It, typename T>
static constexpr bool _find(It& rngBegin, size_t rngSize, const T& val)
{
    if constexpr (can_prefer_std_algorithm<It, T>)
    {
        if (!std::is_constant_evaluated())
        {
            const auto result = _find_std(rngBegin, rngSize, val);
            if (result != 2)
                return result;
        }
    }

    if constexpr (can_memchr<It, T>)
    {
        if (!std::is_constant_evaluated())
            return _find_memchr(rngBegin, rngSize, val);
    }
    else if constexpr (can_memchr_convertible<It, T>)
    {
        if (!std::is_constant_evaluated())
            return _find_memchr_convert(rngBegin, rngSize, val);
    }
    else if constexpr (can_memchr_barelly<It, T>)
    {
        if (!std::is_constant_evaluated())
            return _find_memchr_barelly(rngBegin, rngSize, val);
    }

    return _find_native(rngBegin, rngSize, val);
}

template <typename It, typename T>
constexpr It find(It begin, It end, const T& val)
{
    if (_find(begin, std::distance(begin, end), val))
        return begin;
    return end;
}

template <typename It, typename T>
constexpr It find(It begin, size_t rngSize, const T& val)
{
    auto beginBackup = begin;
    if (_find(begin, rngSize, val))
        return begin;
    return beginBackup + rngSize;
}

template <typename It, same_iter_size<It> It2>
static bool _search_memchr_then_memcmp(It& srcBegin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    static_assert(sizeof(std::iter_value_t<It>) == sizeof(uint8_t));
    static_assert(sizeof(std::iter_value_t<It2>) == sizeof(uint8_t));

    const int firstByte = *targetBegin;

    constexpr auto compareSrcOffset = 1;
    const void*    compareBegin     = static_cast<const uint8_t*>(static_cast<const void*>(targetBegin)) + compareSrcOffset;
    const auto     compareCount     = targetSize - compareSrcOffset;

    for (;;)
    {
        auto found = memchr(srcBegin, firstByte, srcSize - compareCount);
        if (found)
        {
            const auto offset = _seek_to_ex(srcBegin, found);
            if (memcmp(static_cast<const uint8_t*>(static_cast<const void*>(srcBegin)) + compareSrcOffset, compareBegin, compareCount) == 0)
                return true;
            srcSize -= offset;
        }
        --srcSize;
        if (srcSize < targetSize)
            return false;
        ++srcBegin;
    }
}

template <typename It, same_iter_size<It> It2>
static bool _search_memcmp(It& begin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    const void* targetBeginVoid = targetBegin;
    const auto  targetSizeBytes = targetSize * sizeof(std::iter_value_t<It2>);
    for (;;)
    {
        if (memcmp(begin, targetBeginVoid, targetSizeBytes) == 0)
            return true;
        --srcSize;
        if (srcSize < targetSize)
            return false;
        ++begin;
    }
}

template <typename It, typename It2>
static bool _search_native(It& begin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    for (;;)
    {
        auto tempBegin       = begin;
        auto temptargetBegin = targetBegin;
        auto remaining       = targetSize;

        for (;;)
        {
            if (*tempBegin != *temptargetBegin)
            {
                srcSize -= _seek_to_ex(begin, tempBegin + 1);
                if (srcSize < targetSize)
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

#if 0
template <typename It, typename It2>
static bool _search_memcmp_or_native(It& begin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    if constexpr (same_iter_size<It, It2>)
        return _search_memcmp<It, It2>(begin, srcSize, targetBegin, targetSize);
    else
        return _search_native<It, It2>(begin, srcSize, targetBegin, targetSize);
}
#endif

template <typename It, typename It2>
static bool _search_memchr_barelly_then_memcmp(It& srcBegin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    using it_val  = std::iter_value_t<It>;
    using it2_val = std::iter_value_t<It2>;

    static_assert(sizeof(it_val) == sizeof(it2_val));

    const auto& firstTarget     = *targetBegin;
    const auto  firstTargetByte = _read_first_byte(firstTarget);
    if (!firstTargetByte)
        return _search_memcmp<It, It2>(srcBegin, srcSize, targetBegin, targetSize);

    const auto absBegin = reinterpret_cast<uintptr_t>(static_cast<const void*>(srcBegin));

    const void* secondTargetByte       = static_cast<const uint8_t*>(static_cast<const void*>(targetBegin)) + 1;
    const auto  targetBytesCount       = targetSize * sizeof(it2_val);
    const auto  targetBytesCountMinus1 = targetBytesCount - 1;

    for (;;)
    {
        auto found = reinterpret_cast<uintptr_t>(memchr(srcBegin, firstTargetByte, srcSize * sizeof(it_val)));
        if (!found)
            return false;

        const auto offset = (found - absBegin) % sizeof(it_val);
        if (offset != 0)
        {
            found += sizeof(it_val) - offset;
        }
        else if (memcmp(reinterpret_cast<void*>(found + 1), secondTargetByte, targetBytesCountMinus1) == 0)
        {
            _seek_to(srcBegin, reinterpret_cast<void*>(found));
            return true;
        }

        srcSize -= _seek_to_ex(srcBegin, reinterpret_cast<void*>(found));
        if (srcSize < targetSize)
            return false;
    }
}

template <typename It, typename It2>
static bool _search_memchr_convertible_then_memcmp(It& begin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    using it_val  = std::iter_value_t<It>;
    using it2_val = std::iter_value_t<It2>;

    const auto& val = *targetBegin;

    const auto itVal = static_cast<it_val>(val);
    if constexpr (sizeof(it_val) < sizeof(it2_val))
    {
        if (static_cast<it2_val>(itVal) == val)
        {
            if constexpr (can_memchr<It, it_val>)
                return _search_memchr_then_memcmp<It, It2>(begin, srcSize, targetBegin, targetSize);
        }
        else
        {
            return _search_memcmp<It, It2>(begin, srcSize, targetBegin, targetSize);
        }
    }

    return _search_memchr_barelly_then_memcmp<It, It2>(begin, srcSize, targetBegin, targetSize);
}

template <typename It, typename It2>
static uint8_t _search_std_find(It& rngBegin, size_t rngSize, It2 targetBegin, const size_t targetSize)
{
    const auto&           val  = *targetBegin;
    std::iter_value_t<It> val1 = val;
    if constexpr (sizeof(std::iter_value_t<It>) < sizeof(std::iter_value_t<It2>))
    {
        if (static_cast<std::iter_value_t<It2>>(val1) != val)
            return 2;
    }

    for (;;)
    {
        if (rngSize < targetSize)
            return 0;

        auto last  = rngBegin + rngSize;
        It   found = (__std_find_trivial<std::iter_value_t<It>>(rngBegin, last, val1));

        if (!found)
            return 0;

        if constexpr (can_memcmp<It, It2>)
        {
            if (memcmp((found) + 1, targetBegin + 1, (targetSize - 1) * sizeof(std::iter_value_t<It2>)) == 0)
            {
                _seek_to(rngBegin, found);
                return 1;
            }
        }
        else
        {
            It   itL   = found + 1;
            auto itR   = targetBegin + 1;
            auto count = targetSize - 1;
            while (*itL == *itR)
            {
                if (count-- == 0)
                {
                    _seek_to(rngBegin, found);
                    return 1;
                }

                ++itL;
                ++itR;
            }
        }

        rngSize -= _seek_to_ex(rngBegin, found);
    }
}

template <typename It, typename It2>
static constexpr bool _search(It& begin, size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    if (srcSize < targetSize)
        return false;

    using it2_maybe_ref = try_add_ref_t<It2>;
    using it2_val       = std::iter_value_t<It2>;

    if constexpr (can_prefer_std_algorithm<It, it2_val>)
    {
        if (!std::is_constant_evaluated())
        {
            auto result = _search_std_find<It, it2_maybe_ref>(begin, srcSize, targetBegin, targetSize);
            if (result != 2)
                return result;
        }
    }

    if constexpr (can_memchr<It, it2_val>)
    {
        if (!std::is_constant_evaluated())
            return _search_memchr_then_memcmp<It, it2_maybe_ref>(begin, srcSize, targetBegin, targetSize);
    }
    else if constexpr (can_memcmp<It, It2>)
    {
        if (!std::is_constant_evaluated())
        {
            if constexpr (can_memchr_convertible<It, it2_val>)
                return _search_memchr_convertible_then_memcmp<It, it2_maybe_ref>(begin, srcSize, targetBegin, targetSize);
            else if constexpr (can_memchr_barelly<It, it2_val>)
                return _search_memchr_barelly_then_memcmp<It, it2_maybe_ref>(begin, srcSize, targetBegin, targetSize);
            else
                return _search_memcmp<It, it2_maybe_ref>(begin, targetBegin, targetSize);
        }
    }

    return _search_native<It, it2_maybe_ref>(begin, srcSize, targetBegin, targetSize);
}

template <typename It, typename It2>
constexpr It find(It begin, const size_t srcSize, It2 targetBegin, const size_t targetSize)
{
    auto beginBackup = begin;
    if (!_search(begin, srcSize, targetBegin, targetSize))
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
constexpr It find(It begin, It end, It2 targetBegin, const size_t targetSize)
{
    if (!_search(begin, std::distance(begin, end), targetBegin, targetSize))
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