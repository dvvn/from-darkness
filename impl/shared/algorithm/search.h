#pragma once

#include "functional/basic_function.h"

namespace fd
{
using search_stop_token = basic_function<bool, void*>;

struct basic_pattern;
struct basic_xref;

void* search(void* begin, void const* end, basic_pattern const& pattern, search_stop_token const& token);
void* search(void* begin, void const* end, basic_xref const& xref, search_stop_token const& token);
void* search(void* begin, void const* end, void const* from, void const* to, search_stop_token const& token);
} // namespace fd