#pragma once
#include "memory/basic_pattern.h"

namespace fd
{
struct basic_xref;

void* find(void* begin, void const* end, basic_pattern const& pattern);
void* find(void* begin, void const* end, basic_xref const& xref);
void* find(void* begin, void const* end, void const* from, void const* to);

template <size_t... SegmentsBytesCount>
struct pattern;

template <size_t... SegmentsBytesCount>
void* find(void* begin, void const* end, pattern<SegmentsBytesCount...> const& pattern)
{
    return 0; // WIP
}
}