#pragma once

#include "tier1/pattern.h"

namespace FD_TIER(1)
{
template <typename It, pattern_size_type Bytes, pattern_size_type UnknownBytes>
bool equal(It it, pattern_segment<Bytes, UnknownBytes> const& segment)
{
    decltype(auto) segment_view = segment.view();
    return std::equal(ubegin(segment_view), uend(segment_view), it);
}

template <typename It, class... Segment>
bool equal(It first, pattern<Segment...> const& pat)
{
    constexpr auto segment_equal = []<typename T>(T& it, auto& segment) {
        auto const eq = equal<T>(it, segment);
        if (eq)
            it += segment.length();
        return eq;
    };

    if constexpr (std::same_as<It, std::remove_reference_t<decltype(unwrap_iterator(first))>>)
        return boost::hana::unpack(pat.get(), [&first](auto&&... segment) -> bool {
            return (segment_equal(first, segment) && ...);
        });
    else
        return boost::hana::unpack(pat.get(), [u_first = unwrap_iterator(first)](auto&&... segment) -> bool {
            return (segment_equal(u_first, segment) && ...);
        });
}

}