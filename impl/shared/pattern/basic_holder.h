#pragma once

#include <cstddef>

namespace fd
{
#ifdef _DEBUG
using pattern_size_type       = size_t;
using pattern_difference_type = ptrdiff_t;
#else
using pattern_size_type       = uint8_t;
using pattern_difference_type = int8_t;
#endif

template <pattern_size_type BytesCount>
struct pattern_segment_bytes;

template <pattern_size_type BytesCount>
struct pattern_segment_unknown_bytes;

template <pattern_size_type Bytes, pattern_size_type UnknownBytes>
struct pattern_segment;

template <class... Segment>
struct pattern;

template <class Segment>
struct pattern_segment_constant_size;

template <>
struct pattern_segment_constant_size<pattern_segment<-1, -1>>;

template <pattern_size_type UnknownBytes>
struct pattern_segment_constant_size<pattern_segment<-1, UnknownBytes>>;

template <pattern_size_type Bytes>
struct pattern_segment_constant_size<pattern_segment<Bytes, -1>>;

template <pattern_size_type Bytes, pattern_size_type UnknownBytes>
struct pattern_segment_constant_size<pattern_segment<Bytes, UnknownBytes>>
{
    static constexpr std::integral_constant<pattern_size_type, Bytes> known;
    static constexpr std::integral_constant<pattern_size_type, UnknownBytes> unknown ;

    [[deprecated]] //
    static constexpr pattern_size_type size = Bytes + UnknownBytes;
    static constexpr std::integral_constant<pattern_size_type, Bytes + UnknownBytes> length;
};
} // namespace fd