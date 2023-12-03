#pragma once

#include "tier0/iterator/unwrap.h"
#include "tier1/diagnostics/fatal.h"

#include <algorithm>

namespace FD_TIER(0)
{
struct find_callback_gap final
{
};

template <typename Callback, typename V>
bool invoke_find_callback(find_callback_gap callback, V value)
{
    return true;
}

template <typename Callback, typename V>
bool invoke_find_callback(Callback& callback, V value)
{
    if constexpr (std::invocable<Callback, V>)
        return callback(value);
    else
        return true;
}

template <typename Callback, typename V>
bool invoke_find_callback(std::reference_wrapper<Callback> callback, V value)
{
    if constexpr (std::invocable<Callback, V>)
        return callback(value);
    else
        return true;
}

template <bool ReturnDefault, typename It, typename Callback>
It find_byte(It rng_first, It const rng_last, uint8_t const byte, std::reference_wrapper<Callback> callback)
{
    constexpr auto callback_invocable = std::invocable<Callback, decltype(unwrap_iterator(rng_first))>;
    if constexpr (!callback_invocable)
    {
        auto const found = std::find(rng_first, rng_last, byte);
        if (found != rng_last)
            return found;
    }
    else
    {
        verify_range(rng_first, rng_last);

        auto u_rng_first      = unwrap_iterator(rng_first);
        auto const u_rng_last = unwrap_iterator(rng_last);

        for (;;)
        {
            auto const u_rng_byte = std::find(u_rng_first, u_rng_last, byte);
            if (u_rng_byte == u_rng_last)
                break;

            if (!invoke_find_callback(callback, u_rng_byte))
            {
                u_rng_first = u_rng_byte + 1;
                continue;
            }

            rewrap_iterator(rng_first, u_rng_byte);
            return rng_first;
        }
    }

    if constexpr (ReturnDefault)
        return {};
    else
        return rng_last;
}

//-----

template <typename It, typename It2, typename Callback = find_callback_gap>
It find(It rng_first, It const rng_last, It2 const what_first, It2 const what_last, Callback callback = {})
{
    verify_range(what_first, what_last);

    auto const u_what_first = unwrap_iterator(what_first);
    auto const u_what_last  = unwrap_iterator(what_last);

    auto const what_front    = *u_what_first;
    auto const target_length = std::distance(u_what_first, u_what_last);

    // verify_range(rng_first, rng_last);

    if (target_length == 1)
    {
        return find_byte<false>(rng_first, rng_first, what_front, std::ref(callback));
    }

    auto u_rng_first           = unwrap_iterator(rng_first);
    auto const u_rng_safe_last = unwrap_iterator(rng_last - target_length);

    // verify_range(u_rng_first, u_rng_safe_last);

    for (;;)
    {
        auto const u_rng_what_first = std::find(u_rng_first, u_rng_safe_last, what_front);
        if (u_rng_what_first == u_rng_safe_last)
            return rng_last;
        if (!std::equal(u_what_first, u_what_last, u_rng_what_first))
        {
            u_rng_first = rng_first + 1;
            continue;
        }
        if (!invoke_find_callback(callback, u_rng_what_first))
        {
            u_rng_first = u_rng_what_first + target_length;
            continue;
        }
        rewrap_iterator(rng_first, u_rng_what_first);
        return rng_first;
    }
}
} // namespace FD_TIER(0)
