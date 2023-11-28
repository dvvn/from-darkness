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

    static auto call(std::reference_wrapper<Callback> callback, V found)
    {
        if constexpr (invocable_arg)
            return std::invoke(callback, found);
        else if constexpr (invocable_raw)
            return std::invoke(callback);
        else
            return true;
    }
};

template <typename It, typename Callback>
It find_one_byte(It rng_start, It const rng_end, uint8_t const first_value, std::reference_wrapper<Callback> callback)
{
    using cb_invoker1 = find_callback_invoker<Callback, decltype(unwrap_iterator(rng_start))>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    if constexpr (cb_invoker1::invocable || cb_invoker2::invocable)
    {
        verify_range(rng_start, rng_end);

        auto u_rng_start     = unwrap_iterator(rng_start);
        auto const u_rng_end = unwrap_iterator(rng_end);

        for (;;)
        {
            auto pos_found = std::find(u_rng_start, u_rng_end, first_value);
            if (pos_found == u_rng_end)
                return rng_end;

            if (cb_invoker1::invocable ? cb_invoker1::call(callback, u_rng_start) : cb_invoker2::invocable ? cb_invoker2::call(callback, rng_start) : true)
            {
                rewrap_iterator(rng_start, pos_found);
                return rng_start;
            }

            u_rng_start = pos_found + 1;
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

    auto const u_what_start = unwrap_iterator(what_start);
    auto const u_what_end   = unwrap_iterator(what_end);

    auto const what_front    = *u_what_start;
    auto const target_length = std::distance(u_what_start, u_what_end);

    // verify_range(rng_start, rng_end);

    auto const callback_ref = std::ref(callback);

    if (target_length == 1)
    {
        return detail::find_one_byte(rng_start, rng_start, what_front, callback_ref);
    }

    auto u_rng_start          = unwrap_iterator(rng_start);
    auto const u_rng_safe_end = unwrap_iterator(rng_end - target_length);

    // verify_range(u_rng_start, u_rng_safe_end);

    using cb_invoker1 = detail::find_callback_invoker<Callback, decltype(u_rng_start)>;
    using cb_invoker2 = detail::find_callback_invoker<Callback, It>;

    for (;;)
    {
        auto const front_found = std::find(u_rng_start, u_rng_safe_end, what_front);
        if (front_found == u_rng_safe_end)
            return rng_end;
        if (!std::equal(u_what_start, u_what_end, u_rng_start))
        {
            u_rng_start = rng_start + 1;
            continue;
        }
        if (cb_invoker1::invocable ? cb_invoker1::call(callback, u_rng_start) : cb_invoker2::invocable ? cb_invoker2::call(callback, rng_start) : true)
        {
            rewrap_iterator(rng_start, front_found);
            return rng_start;
        }
        u_rng_start = front_found + target_length;
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