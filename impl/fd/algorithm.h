#pragma once

#include <fd/views.h>

namespace fd
{
template <typename T, size_t MaxSize = sizeof(void*) * 2>
using try_ref_t = std::conditional_t<!std::is_lvalue_reference_v<T> && (!std::is_trivially_copyable_v<T> || sizeof(T) > MaxSize), std::add_lvalue_reference_t<T>, T>;

template <typename It, typename T>
static constexpr auto _to_iter_value(const T& val)
{
    return static_cast<std::iter_value_t<It>>(val);
}

template <typename It, typename Fn>
static constexpr void _iterate(It begin, It end, Fn fn)
{
    for (; begin != end; ++begin)
    {
        fn(*begin);
    }
}

template <typename It, typename Fn>
static constexpr void _iterate(It begin, size_t rngSize, Fn fn)
{
    for (;;)
    {
        fn(*begin);
        if (--rngSize == 0)
            return;
        ++begin;
    }
}

template <typename It, typename Fn>
constexpr void iterate(It begin, It end, Fn fn)
{
    _iterate<try_ref_t<Fn>>(decay_iter(begin), decay_iter(end), fn);
}

template <native_iterable T, typename Fn>
constexpr void iterate(T&& rng, Fn fn)
{
    _iterate<try_ref_t<Fn>>(_begin(rng), _size_or_end(rng), fn);
}

template <typename It, typename It2>
static constexpr void _copy(It src, It end, It2 dst)
{
    for (;;)
    {
        *dst = *src;
        if (++src == end)
            return;
        ++dst;
    }
}

template <typename It, typename It2>
static constexpr void _copy(It src, size_t srcSize, It2 dst)
{
    for (;;)
    {
        *dst = *src;
        if (--srcSize == 0)
            return;
        ++src;
        ++dst;
    }
}

template <typename Src, typename Dst>
constexpr void copy(Src begin, Src end, Dst dst)
{
    _copy(decay_iter(begin), decay_iter(end), decay_iter(dst));
}

template <typename It, typename It2>
constexpr void copy(It src, size_t srcSize, It2 dst)
{
    _copy(decay_iter(src), srcSize, decay_iter(dst));
}

template <native_iterable T, typename It2>
constexpr void copy(T&& rng, It2 dst)
{
    _copy(_begin(rng), _size_or_end(rng), decay_iter(dst));
}

template <typename It, typename It2>
static constexpr bool _equal(It begin, It end, It2 with)
{
    for (;;)
    {
        if (*begin != *with)
            return false;
        if (++begin == end)
            return true;
        ++with;
    }
}

template <typename It, typename It2>
static constexpr bool _equal(It begin, size_t rngSize, It2 with)
{
    for (;;)
    {
        if (*begin != *with)
            return false;
        if (--rngSize == 0)
            return true;
        ++begin;
        ++with;
    }
}

// template <typename T, typename It2>
// static constexpr bool _equal_range(T&& rng, It2 with)
//{
//     return _equal<decltype(rng.begin()), try_ref_t<It2>>(rng.begin(), rng.end(), with);
// }

template <typename It, typename It2>
constexpr bool equal(It begin, It end, It2 with)
{
    return _equal(decay_iter(begin), decay_iter(end), decay_iter(with));
}

template <typename It, typename It2>
constexpr bool equal(It begin, size_t rngSize, It2 with)
{
    return _equal(decay_iter(begin), rngSize, decay_iter(with));
}

template <native_iterable T, typename It2>
constexpr bool equal(T&& rng, It2 with)
{
    return _equal(_begin(rng), _size_or_end(rng), decay_iter(with));
}

template <typename It, typename T>
static constexpr void _fill(It begin, It end, const T val)
{
    for (; begin != end; ++begin)
    {
        *begin = val;
    }
}

template <typename It, typename T>
static constexpr void _fill(It begin, size_t rngSize, const T val)
{
    for (;;)
    {
        *begin = val;
        if (--rngSize == 0)
            return;
        ++begin;
    }
}

template <typename It, typename T>
constexpr void fill(It begin, It end, const T& val)
{
    _fill(decay_iter(begin), decay_iter(end), _to_iter_value<It>(val));
}

template <typename It, typename T>
constexpr void fill(It begin, size_t rngCount, const T& val)
{
    _fill(decay_iter(begin), rngCount, _to_iter_value<It>(val));
}

template <native_iterable T, typename T1>
constexpr void fill(T& rng, const T1& val)
{
    _fill(_begin(rng), _size_or_end(rng), _to_iter_value<iter_t<T>>(val));
}

template <typename It, typename T>
static constexpr It _find(It begin, It end, const T val)
{
    for (; begin != end; ++begin)
    {
        if (*begin == val)
            return begin;
    }
    return end;
}

template <typename It, typename T>
static constexpr It _find(It begin, size_t rngSize, const T val)
{
    for (;;)
    {
        if (*begin == val)
            break; // found
        ++begin;
        if (--rngSize == 0)
            break; // end
    }

    return begin;
}

template <typename V, typename It>
concept can_find_value_inside = requires(It it, V val) {
                                    *it == val;
                                    *it == _to_iter_value<It>(val);
                                };

template <typename It, can_find_value_inside<It> T>
constexpr It find(It begin, It end, const T& val)
{
    return _find(decay_iter(begin), decay_iter(end), _to_iter_value<It>(val));
}

template <typename It, can_find_value_inside<It> T>
constexpr It find(It begin, size_t rngSize, const T& val)
{
    return _find(decay_iter(begin), rngSize, _to_iter_value<It>(val));
}

template <native_iterable T, can_find_value_inside<iter_t<T>> V>
constexpr iter_t<T> find(T&& rng, const V& val)
{
    return _find(_begin(rng), _size_or_end(rng), _to_iter_value<iter_t<T>>(val));
}

// template <typename It>
// static auto _rewrap_range(It begin, size_t rngSize)
//{
//     using it_val = std::iter_value_t<It>;
//     if constexpr (std::integral<it_val>)
//     {
//         const auto sizeBytes = rngSize * sizeof(it_val);
//     }
//     else
//     {
//         return std::pair(begin, rngSize);
//     }
// }

template <typename It, typename It2>
static constexpr It _find_range(It begin, It end, It2 begin2, It2 end2)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (const auto last = end - std::distance(begin2, end2) + 1; begin != last; ++begin)
    {
        auto absBegin = begin;
        if (_equal<it2_eq, it_eq>(begin2, end2, begin))
            return absBegin;
    }

    return end;
}

template <typename It, typename It2>
static constexpr It _find_range(It begin, size_t rngSize, It2 begin2, It2 end2)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    const auto testSize = std::distance(begin2, end2);
    for (const auto limit = rngSize - testSize + 1; limit != 0; --limit)
    {
        auto absBegin = begin;
        if (_equal<it2_eq, it_eq>(begin2, end2, begin))
            return absBegin;
    }
    return begin + testSize;
}

template <typename It, typename It2>
static constexpr It _find_range(It begin, It end, It2 begin2, size_t testSize)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (const auto last = end - testSize + 1; begin != last; ++begin)
    {
        auto absBegin = begin;
        if (_equal<it2_eq, it_eq>(begin2, testSize, begin))
            return absBegin;
    }

    return end;
}

template <typename It, typename It2>
static constexpr It _find_range(It begin, size_t rngSize, It2 begin2, size_t testSize)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (auto limit = rngSize - testSize + 1; limit != 0; --limit)
    {
        auto absBegin = begin;
        if (_equal<it2_eq, it_eq>(begin2, testSize, begin))
            return absBegin;
    }

    return begin + testSize;
}

template <typename It, typename It2>
constexpr It find(It begin, It end, It2 begin2, It2 end2)
{
    return _find_range(decay_iter(begin), decay_iter(end), decay_iter(begin2), decay_iter(end2));
}

template <typename It, typename It2>
constexpr It find(It begin, size_t rngSize, It2 begin2, It2 end2)
{
    return _find_range(decay_iter(begin), rngSize, decay_iter(begin2), decay_iter(end2));
}

template <typename It, typename It2>
constexpr It find(It begin, It end, It2 begin2, size_t testSize)
{
    return _find_range(decay_iter(begin), decay_iter(end), decay_iter(begin2), testSize);
}

template <typename It, typename It2>
constexpr It find(It begin, size_t rngSize, It2 begin2, size_t testSize)
{
    return _find_range(decay_iter(begin), rngSize, decay_iter(begin2), testSize);
}

template <native_iterable Rng, typename It2>
constexpr iter_t<Rng> find(Rng&& rng, It2 begin2, It2 end2)
{
    return _find_range(_begin(rng), _size_or_end(rng), decay_iter(begin2), decay_iter(end2));
}

template <native_iterable Rng, typename It2>
constexpr iter_t<Rng> find(Rng&& rng, It2 begin2, size_t testSize)
{
    return _find_range(_begin(rng), _size_or_end(rng), decay_iter(begin2), testSize);
}

template <typename Rng, typename It>
concept can_find_range_inside = (!can_find_value_inside<Rng, It>) && can_find_value_inside<std::iter_value_t<iter_t<Rng>>, It>;

template <typename It, can_find_range_inside<It> Rng>
constexpr It find(It begin, It end, Rng&& testRng)
{
    return _find_range(decay_iter(begin), decay_iter(end), _begin(testRng), _size_or_end(testRng));
}

template <typename It, can_find_range_inside<It> Rng>
constexpr It find(It begin, size_t rngSize, Rng&& testRng)
{
    return _find_range(decay_iter(begin), rngSize, _begin(testRng), _size_or_end(testRng));
}

template <typename TestRng, typename Rng>
concept can_find_range_inside_range = (!can_find_value_inside<TestRng, Rng>) && can_find_value_inside<std::iter_value_t<iter_t<TestRng>>, iter_t<Rng>>;

template <native_iterable T, can_find_range_inside_range<T> T2>
constexpr iter_t<T> find(T&& rng, T2&& testRng)
{
    return _find_range(_begin(rng), _size_or_end(rng), _begin(testRng), _size_or_end(testRng));
}

template <typename It, typename T>
static constexpr bool _contains(It begin, It end, const T val)
{
    for (; begin != end; ++begin)
    {
        if (*begin == val)
            return true;
    }
    return false;
}

template <typename It, typename T>
static constexpr bool _contains(It begin, size_t rngSize, const T val)
{
    for (;;)
    {
        if (*begin == val)
            return true;
        if (--rngSize == 0)
            return false;
        ++begin;
    }
}

template <typename It, typename It2>
static constexpr bool _contains_range(It begin, It end, It2 begin2, It2 end2)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (const auto last = end - std::distance(begin2, end2) + 1; begin != last; ++begin)
    {
        if (_equal<it2_eq, it_eq>(begin2, end2, begin))
            return true;
    }

    return false;
}

template <typename It, typename It2>
static constexpr bool _contains_range(It begin, size_t rngSize, It2 begin2, It2 end2)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (auto limit = rngSize - std::distance(begin2, end2) + 1; limit != 0; --limit)
    {
        if (_equal<it2_eq, it_eq>(begin2, end2, begin))
            return true;
    }

    return false;
}

template <typename It, typename It2>
static constexpr bool _contains_range(It begin, It end, It2 begin2, size_t testSize)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (const auto last = end - testSize + 1; begin != last; ++begin)
    {
        if (_equal<it2_eq, it_eq>(begin2, testSize, begin))
            return true;
    }

    return false;
}

template <typename It, typename It2>
static constexpr bool _contains_range(It begin, size_t rngSize, It2 begin2, size_t testSize)
{
    using it_eq  = std::add_lvalue_reference_t<It>;
    using it2_eq = std::remove_reference_t<It2>;

    for (auto limit = rngSize - testSize + 1; limit != 0; --limit)
    {
        if (_equal<it2_eq, it_eq>(begin2, testSize, begin))
            return true;
    }

    return false;
}

template <typename It, typename It2>
constexpr bool contains(It begin, It end, It2 begin2, It2 end2)
{
    return _contains_range(decay_iter(begin), decay_iter(end), decay_iter(begin2), decay_iter(end2));
}

template <typename It, typename It2>
constexpr bool contains(It begin, size_t rngSize, It2 begin2, It2 end2)
{
    return _contains_range(decay_iter(begin), rngSize, decay_iter(begin2), decay_iter(end2));
}

template <typename It, typename It2>
constexpr bool contains(It begin, It end, It2 begin2, size_t testSize)
{
    return _contains_range(decay_iter(begin), decay_iter(end), decay_iter(begin2), testSize);
}

template <typename It, typename It2>
constexpr bool contains(It begin, size_t rngSize, It2 begin2, size_t testSize)
{
    return _contains_range(decay_iter(begin), rngSize, decay_iter(begin2), testSize);
}

template <native_iterable Rng, typename It2>
constexpr bool contains(Rng&& rng, It2 begin2, It2 end2)
{
    return _contains_range(_begin(rng), _size_or_end(rng), decay_iter(begin2), decay_iter(end2));
}

template <native_iterable Rng, typename It2>
constexpr bool contains(Rng&& rng, It2 begin2, size_t testSize)
{
    return _contains_range(_begin(rng), _size_or_end(rng), decay_iter(begin2), testSize);
}

template <typename It, can_find_range_inside<It> Rng>
constexpr bool contains(It begin, It end, Rng&& testRng)
{
    return _contains_range(decay_iter(begin), decay_iter(end), _begin(testRng), _size_or_end(testRng));
}

template <typename It, can_find_range_inside<It> Rng>
constexpr bool contains(It begin, size_t rngSize, Rng&& testRng)
{
    return _contains_range(decay_iter(begin), rngSize, _begin(testRng), _size_or_end(testRng));
}

template <native_iterable T, can_find_range_inside_range<T> T2>
constexpr bool contains(T&& rng, T2&& testRng)
{
    return _contains_range(_begin(rng), _size_or_end(rng), _begin(testRng), _size_or_end(testRng));
}

class console_writer_front;
size_t test_algorithms(const console_writer_front&);
}