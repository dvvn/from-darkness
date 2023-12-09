#pragma once

#include "type_traits.h"

// #define FD_NATIVE_PATTERN_SIZE

namespace fd
{
template <size_t BytesCount>
struct pattern_segment_bytes;

template <size_t BytesCount>
struct pattern_segment_unknown_bytes;

template <size_t Bytes, size_t UnknownBytes>
struct pattern_segment;

template <class... Segment>
class pattern;

template <class Segment>
class pattern_segment_constant_size;

template <>
class pattern_segment_constant_size<pattern_segment<-1, -1>>;

template <size_t UnknownBytes>
class pattern_segment_constant_size<pattern_segment<-1, UnknownBytes>>;

template <size_t Bytes>
class pattern_segment_constant_size<pattern_segment<Bytes, -1>>;

template <size_t Bytes, size_t UnknownBytes>
class pattern_segment_constant_size<pattern_segment<Bytes, UnknownBytes>>
{
#ifdef FD_NATIVE_PATTERN_SIZE
    template <size_t S>
    using integral_constant = integral_constant<size_t, S>;
#else
    template <size_t S>
    using integral_constant = detail::small_integral_constant<size_t, S>;
#endif
  public:
    static constexpr integral_constant<Bytes> known;
    static constexpr integral_constant<UnknownBytes> unknown;

    static constexpr integral_constant<Bytes + UnknownBytes> length;
};
} // namespace fd