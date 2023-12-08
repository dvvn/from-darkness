#pragma once

#include "pattern/holder.h"

namespace fd
{
template <typename It, pattern_size_type BytesCount>
bool equal(pattern_segment_bytes<BytesCount> const& bytes, It it)
{
    auto const first  = bytes.data();
    auto const length = bytes.size();
    return std::equal(first, first + length, it);
}

template <typename It, pattern_size_type Bytes, pattern_size_type UnknownBytes>
bool equal(pattern_segment<Bytes, UnknownBytes> const& segment, It it)
{
#ifdef _DEBUG
    if constexpr (UnknownBytes != 0 && std::is_class_v<It>)
        std::next(it, segment.length());
#endif
    return equal(segment.get(), it);
}

template <typename It, class... Segment>
bool equal(pattern<Segment...> const& pat, It it)
{
    if constexpr (sizeof...(Segment) == 1)
    {
        return equal(pat.template get<0>(), it);
    }
    else
    {
        auto const equal_fn = [it](auto& self, auto offset, auto& segment, auto&... next) -> bool {
            if (!equal(segment, it + offset))
                return false;
            if constexpr (sizeof...(next) == 0)
                return true;
            else
                return self(self, offset + segment.known() + segment.unknown(), next...);
        };
        constexpr auto constant_size = (complete<pattern_segment_constant_size<Segment>> && ...);
        return boost::hana::unpack(pat.get(), [&equal_fn](Segment const&... segment) -> bool {
#if 0
            return equal_fn(equal_fn, std::conditional_t<constant_size, std::integral_constant<pattern_size_type, 0>, pattern_size_type>{}, segment...);
#else
            if constexpr (constant_size)
                return equal_fn(equal_fn, std::integral_constant<pattern_size_type, 0>{}, segment...);
            else
                return equal_fn(equal_fn, static_cast<pattern_size_type>(0), segment...);
#endif
        });
    }
}
} // namespace fd