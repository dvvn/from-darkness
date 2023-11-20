#pragma once
#include "iterator/unwrap.h"

#include <algorithm>

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

    static auto call(Callback& callback, V found)
    {
        if constexpr (invocable_arg)
            return std::invoke(callback, found);
        else if constexpr (invocable_raw)
            return std::invoke(callback);
    }
};

template <typename It, typename ItRaw, typename Callback>
bool invoke_find_callback(It& first_pos, ItRaw current_pos, std::reference_wrapper<Callback> callback)
{
    using cb_invoker  = find_callback_invoker<Callback, ItRaw>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    if constexpr (cb_invoker::invocable)
    {
        if (!cb_invoker::call(callback, current_pos))
            return false;
        rewrap_iterator(first_pos, current_pos);
    }
    else if constexpr (cb_invoker2::invocable)
    {
        rewrap_iterator(first_pos, current_pos);
        if (!cb_invoker2::call(callback, first_pos))
            return false;
    }
    else
    {
        rewrap_iterator(first_pos, current_pos);
    }

    return true;
}

template <typename It, typename Callback>
It find_one_byte(It rng_start, It const rng_end, uint8_t const first_value, std::reference_wrapper<Callback> callback)
{
    using cb_invoker  = find_callback_invoker<Callback, decltype(unwrap_iterator(rng_start))>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    if constexpr (cb_invoker::invocable || cb_invoker2::invocable)
    {
        verify_range(rng_start, rng_end);

        auto u_rng_start     = unwrap_iterator(rng_start);
        auto const u_rng_end = unwrap_iterator(rng_end);

        for (;;)
        {
            auto pos_found = std::find(u_rng_start, u_rng_end, first_value);
            if (pos_found == u_rng_end)
                return rng_end;

            if (invoke_find_callback(rng_start, pos_found, callback))
                return rng_start;

            u_rng_start = pos_found;
        }
    }
    else
    {
        return std::find(rng_start, rng_end, first_value);
    }
}
} // namespace detail

template <typename It, typename It2 = It, typename Callback = uint8_t>
It find(It rng_start, It const rng_end, It2 const what_start, It2 const what_end, Callback callback = {})
{
    verify_range(what_start, what_end);

    auto const callback_ref = std::ref(callback);

    auto const u_what_start = unwrap_iterator(what_start);
    auto const u_what_end   = unwrap_iterator(what_end);

    auto const what_front    = *u_what_start;
    auto const target_length = std::distance(u_what_start, u_what_end);

    if (target_length == 1)
    {
        return detail::find_one_byte(rng_start, rng_start, what_front, callback_ref);
    }
    // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
    else
    {
        verify_range(rng_start, rng_end);

        auto u_rng_start          = unwrap_iterator(rng_start);
        auto const u_rng_safe_end = unwrap_iterator(rng_end - target_length);

        verify_range(u_rng_start, u_rng_safe_end);

        for (;;)
        {
            auto const front_found = std::find(u_rng_start, u_rng_safe_end, what_front);
            if (front_found == u_rng_safe_end)
                return rng_end;

            if (!std::equal(u_what_start, u_what_end, u_rng_start))
                ++u_rng_start;
            else if (detail::invoke_find_callback(rng_start, front_found, callback_ref))
                return rng_start;
            u_rng_start = front_found + target_length;
        }
    }
}

template <bool Owned>
struct xref;

template <typename Callback = uint8_t, typename It, bool Owned>
auto find(It first, It const last, xref<Owned> const& xr, Callback callback = {})
{
    return find(first, last, xr.begin(), xr.end(), std::ref(callback));
}
}