#pragma once

#include "tier0/algorithm/find.h"
#include "tier1/algorithm/equal_pattern.h"

#include <boost/hana/front.hpp>

namespace FD_TIER(1)
{
template <typename Callback = find_callback_gap, typename It, class... Segment>
It find(It rng_start, It const rng_end, pattern<Segment...> const& pat, Callback callback = {})
{
    auto& first_pattern_segment   = boost::hana::front(pat.get());
    auto const first_pattern_byte = *ubegin(first_pattern_segment);
    auto const callback_ref       = std::ref(callback);

    using std::size;

    if constexpr (sizeof...(Segment) == 1)
    {
        if (size(first_pattern_segment) == 1)
            return find_one_byte(rng_start, rng_end, first_pattern_byte, callback_ref);
    }

    verify_range(rng_start, rng_end);

    auto const pat_size = size(pat);

    auto u_rng_start          = unwrap_iterator(rng_start);
    auto const u_rng_end_safe = unwrap_iterator(rng_end - pat_size);

    // verify_range(u_rng_start, u_rng_end_safe);

    using cb_invoker1 = find_callback_invoker<Callback, decltype(u_rng_start)>;
    using cb_invoker2 = find_callback_invoker<Callback, It>;

    for (;;)
    {
        auto const first_byte_found = std::find(u_rng_start, u_rng_end_safe, first_pattern_byte);
        if (first_byte_found == u_rng_end_safe)
            return rng_end;

        if (!equal(first_byte_found, pat))
        {
            u_rng_start = first_byte_found + 1;
            continue;
        }
        if (cb_invoker1::invocable ? cb_invoker1::call(callback_ref, u_rng_start) : cb_invoker2::invocable ? cb_invoker2::call(callback_ref, rng_start) : true)
        {
            rewrap_iterator(rng_start, first_byte_found);
            return rng_start;
        }

        u_rng_start = first_byte_found + pat_size;
    }
}

}