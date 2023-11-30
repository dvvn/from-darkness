#pragma once

#include "tier1/memory/pattern.h"

// ReSharper disable once CppUnusedIncludeDirective
#include <boost/hana/unpack.hpp>

namespace FD_TIER(1)
{
template <typename It, detail::pattern_size_type Bytes, detail::pattern_size_type UnknownBytes, typename... Next>
bool equal(It mem, pattern_segment<Bytes, UnknownBytes> const& segment, Next const&... next)
{
    using std::begin;
    using std::end;

    auto const ok = std::equal(ubegin(segment), uend(segment), mem);

    if constexpr (sizeof...(Next) == 0)
        return ok;
    else
        return ok && equal(mem + abs_size(segment), next...);
}

template <typename It, class... Segment>
bool equal(It u_start, pattern<Segment...> const& pat)
{
    return boost::hana::unpack(pat.get(), [u_start](auto&... segment) -> bool {
        return equal<It>(u_start, segment...);
    });
}
}