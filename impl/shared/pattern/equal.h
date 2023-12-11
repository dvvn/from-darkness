#pragma once

#include "pattern/holder.h"

namespace fd
{
template <typename It, size_t BytesCount>
bool equal(pattern_segment_bytes<BytesCount> const& bytes, It it)
{
    auto const first  = bytes.data();
    auto const length = bytes.size();
    return std::equal(first, first + length, it);
}

template <typename It, size_t Bytes, size_t UnknownBytes>
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
        return boost::hana::unpack(pat.get(), [&equal_fn](Segment const&... segment) -> bool {
            return equal_fn(equal_fn, 0_c, segment...);
        });
    }
}
} // namespace fd