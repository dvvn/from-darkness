#pragma once

#include "tier0/algorithm/find.h"
#include "tier1/pattern/equal.h"

namespace FD_TIER(1)
{
template <typename Callback = find_callback_gap, typename It, class... Segment>
It find(It rng_first, It const rng_last, pattern<Segment...> const& pat, Callback callback = {})
{
    decltype(auto) front_pattern_segment = pat.front().view();
    auto const front_pattern_byte        = front_pattern_segment.front();

    if constexpr (sizeof...(Segment) == 1)
    {
        if (front_pattern_segment.size() == 1)
            return find_byte<false>(rng_first, rng_last, front_pattern_byte, std::ref(callback));
    }

    verify_range(rng_first, rng_last);

    auto const pattern_length = pat.size();

    auto u_rng_first           = unwrap_iterator(rng_first);
    auto const u_rng_last_safe = unwrap_iterator(rng_last) - pattern_length;

    // verify_range(u_rng_first, u_rng_last_safe);

    for (;;)
    {
        auto const u_rng_front_pattern_byte = std::find(u_rng_first, u_rng_last_safe, front_pattern_byte);
        if (u_rng_front_pattern_byte == u_rng_last_safe)
            return rng_last;
        if (!equal(u_rng_front_pattern_byte, pat))
        {
            u_rng_first = u_rng_front_pattern_byte + 1;
            continue;
        }
        if (!invoke_find_callback(callback, u_rng_front_pattern_byte))
        {
            u_rng_first = u_rng_front_pattern_byte + pattern_length;
            continue;
        }
        rewrap_iterator(rng_first, u_rng_front_pattern_byte);
        return rng_first;
    }
}
}