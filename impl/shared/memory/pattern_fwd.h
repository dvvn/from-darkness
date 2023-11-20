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

template <detail::pattern_size_type... SegmentsBytesCount>
struct pattern;

} // namespace fd