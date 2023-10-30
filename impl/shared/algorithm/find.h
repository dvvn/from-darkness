#pragma once
#include "optional.h"
#include "functional/cast.h"
#include "iterator/unwrap.h"
#include "memory/basic_pattern.h"

#include <boost/hana/front.hpp>
#include <boost/hana/unpack.hpp>

#include <algorithm>
#include <cassert>

namespace fd
{
namespace detail
{
template <typename Callback, typename V>
class find_callback_invoker
{
    static constexpr bool invocable_arg = std::invocable<Callback, V>;
    static constexpr bool invocable_raw = std::invocable<Callback>;

  public:
    static constexpr bool invocable = invocable_arg || invocable_raw;

    static auto invoke(Callback& callback, V found)
    {
        if constexpr (invocable_arg)
            return std::invoke(callback, found);
        else if constexpr (invocable_raw)
            return std::invoke(callback);
    }
};

template <typename It>
auto unwrap_byte_iter(It it)
{
    if constexpr (std::same_as<void*, std::decay_t<It>>)
    {
        return safe_cast<uint8_t>(it);
    }
    else
    {
#ifdef _DEBUG
        static_assert(sizeof(std::iter_value_t<It>) == sizeof(uint8_t));
#endif
        return unwrap_iterator(it);
    }
}

template <typename It, typename ItRaw, typename Callback>
bool invoke_find_callback(It& first_pos, ItRaw current_pos, Callback callback)
{
    using cb_invoker  = find_callback_invoker<Callback, ItRaw>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    if constexpr (cb_invoker::invocable)
    {
        if (!cb_invoker::invoke(callback, current_pos))
            return false;
        rewrap_iterator(first_pos, current_pos);
    }
    else if constexpr (cb_invoker2::invocable)
    {
        rewrap_iterator(first_pos, current_pos);
        if (!cb_invoker2::invoke(callback, first_pos))
            return false;
    }
    else
    {
        rewrap_iterator(first_pos, current_pos);
    }

    return true;
}

template <typename It, typename Callback>
It find_one_byte(It rng_start, It const rng_end, uint8_t const first_value, Callback callback)
{
    using cb_invoker  = find_callback_invoker<Callback, decltype(unwrap_byte_iter(rng_start))>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    if constexpr (cb_invoker::invocable || cb_invoker2::invocable)
    {
        auto u_rng_start     = unwrap_byte_iter(rng_start);
        auto const u_rng_end = unwrap_byte_iter(rng_end);

        for (;;)
        {
            auto pos_found = std::find(u_rng_start, u_rng_end, first_value);
            if (pos_found == u_rng_end)
                return rng_end;

            if (invoke_find_callback(rng_start, pos_found, std::ref(callback)))
                return rng_start;

            u_rng_start = pos_found;
        }
    }
    else
    {
        return (std::find(rng_start, (rng_end), first_value));
    }
}
} // namespace detail

template <typename It, typename It2 = It, typename Callback = uint8_t>
It find(It rng_start, It const rng_end, It2 const what_start, It2 const what_end, Callback callback = {})
{
    using detail::unwrap_byte_iter;

    auto const u_what_start = unwrap_byte_iter(what_start);
    auto const u_what_end   = unwrap_byte_iter(what_end);

    auto const what_front = *u_what_start;

    auto const target_length = std::distance(u_what_start, u_what_end);
    if (target_length == 1)
    {
        return detail::find_one_byte(rng_start, rng_start, what_front, std::ref(callback));
    }
    // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
    else
    {
        auto u_rng_start          = unwrap_byte_iter(rng_start);
        auto const u_rng_end      = unwrap_byte_iter((rng_end));
        auto const u_rng_safe_end = u_rng_end - target_length;
        for (;;)
        {
            auto front_found = std::find(u_rng_start, u_rng_safe_end, what_front);
            if (front_found == u_rng_safe_end)
                return rng_end;

            if (!std::equal(u_what_start, u_what_end, u_rng_start))
                ++u_rng_start;
            else if (detail::invoke_find_callback(rng_start, front_found, std::ref(callback)))
                return rng_start;
            u_rng_start = front_found + target_length;
        }
    }
}

template <detail::pattern_size_type BytesCount>
class pattern_segment;
template <detail::pattern_size_type... SegmentsBytesCount>
struct pattern;

template <typename It, detail::pattern_size_type BytesCount, typename... Next>
bool equal(It mem, pattern_segment<BytesCount> const& segment, Next const&... next)
{
    auto const ok = segment.equal(mem);

    if constexpr (sizeof...(Next) == 0)
        return ok;
    else
        return ok && equal(mem + segment.abs_size(), next...);
}

template <typename It, detail::pattern_size_type... SegmentsBytesCount>
bool equal(It first, It const last, pattern<SegmentsBytesCount...> const& pat)
{
    assert(std::distance(first, last) <= pat.size());
    return boost::hana::unpack(pat.bytes, [u_start = unwrap_iterator(first)](auto&... segment) -> bool {
        return equal(u_start, segment...);
    });
}

template <typename Callback = uint8_t, typename It, detail::pattern_size_type... SegmentsBytesCount>
It find(It rng_start, It const rng_end, pattern<SegmentsBytesCount...> const& pat, Callback callback = {})
{
    auto const first_pattern_byte = boost::hana::front(pat.bytes).front();

    if constexpr (sizeof...(SegmentsBytesCount) == 1)
    {
        return detail::find_one_byte(rng_start, rng_end, first_pattern_byte, std::ref(callback));
    }
    else
    {
        auto u_rng_start     = unwrap_iterator(rng_start);
        auto const u_rng_end = unwrap_iterator(rng_end);

        auto const pat_size       = pat.size();
        auto const u_rng_end_safe = u_rng_end - pat_size;

        for (;;)
        {
            auto const first_byte_found = std::find(u_rng_start, u_rng_end_safe, first_pattern_byte);
            if (first_byte_found == u_rng_end_safe)
                return rng_end;
            if (!equal(first_byte_found, u_rng_end_safe, pat))
                ++u_rng_start;
            else if (detail::invoke_find_callback(rng_start, first_byte_found, std::ref(callback)))
                return rng_start;
            u_rng_start = first_byte_found + pat_size;
        }
    }
}

// template <typename Callback = uint8_t, detail::pattern_size_type... SegmentsBytesCount>
// void* find(void* first, void const* last, pattern<SegmentsBytesCount...> const& pat, Callback callback = {})
//{
//     return find(static_cast<uint8_t*>(first), static_cast<uint8_t const*>(last), pat, std::ref(callback));
// }

template <bool Owned>
struct xref;

template <typename Callback = uint8_t, typename It, bool Owned>
auto find(It first, It const last, xref<Owned> const& xr, Callback callback = {})
{
    return find(first, last, xr.begin(), xr.end(), std::ref(callback));
}
}