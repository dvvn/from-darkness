#pragma once

#include <cstddef>

namespace fd
{
namespace detail
{
#ifdef _DEBUG
using pattern_size_type       = size_t;
using pattern_difference_type = ptrdiff_t;
#else
using pattern_size_type       = uint8_t;
using pattern_difference_type = int8_t;
#endif
} // namespace detail

template <detail::pattern_size_type BytesCount, detail::pattern_size_type UnknownBytesCount>
class pattern_segment;
template <class ...Segment>
struct pattern;

} // namespace fd