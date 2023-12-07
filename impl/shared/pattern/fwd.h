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

template <pattern_size_type Bytes, pattern_size_type UnknownBytes>
struct pattern_segment;

template <class... Segment>
struct pattern;
}