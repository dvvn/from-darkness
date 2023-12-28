#pragma once

#include "pattern/equal.h"

namespace fd
{
template <typename It, class... Segment>
It find(It rng_first, It const rng_last, pattern<Segment...> const& pat) noexcept
{
    using std::find;

    auto const& front_segment = pat.template get<0>();
    auto const& front_bytes   = front_segment.get();
    auto const front_byte     = front_bytes.front();

    if constexpr (sizeof...(Segment) == 1)
    {
        if (front_segment.known() == 1)
            return find(rng_first, rng_last, front_byte);
    }
    auto const pattern_length = pat.length();
    auto const rng_last_safe  = rng_last - pattern_length;

    using std::equal;

    if (rng_first < rng_last_safe)
        for (;;)
        {
            auto const front_byte_found = find(rng_first, rng_last_safe, front_byte);
            if (equal(pat, front_byte_found))
                return front_byte_found;
            if (front_byte_found == rng_last_safe)
                return rng_last;

            rng_first = front_byte_found + 1;
        }

    if (rng_first == rng_last_safe)
        if (equal(pat, rng_first))
            return rng_first;

    return rng_last;
}

template <typename It, class... Segment>
It find_pattern(It rng_first, It const rng_last, pattern<Segment...> const& pat) noexcept
{
    return find(rng_first, rng_last, pat);
}
} // namespace fd