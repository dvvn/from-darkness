#pragma once

#define FD_PATTERN_NATIVE_SIZE

#ifdef FD_PATTERN_NATIVE_SIZE
#include "type_traits/integral_constant.h"
#else
#include "type_traits/small_type.h"
#endif

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
#ifdef FD_PATTERN_NATIVE_SIZE
    template <size_t S>
    using integral_constant = integral_constant<size_t, S>;
#else
    template <size_t S>
    using integral_constant = integral_constant<small_type<size_t, S>, S>;
#endif
  public:
    static constexpr integral_constant<Bytes> known;
    static constexpr integral_constant<UnknownBytes> unknown;

    static constexpr integral_constant<Bytes + UnknownBytes> length;
};
} // namespace fd