#pragma once

namespace fd
{
struct basic_pattern;
struct basic_xref;

void *find(void *begin, void const *end, basic_pattern const &pattern);
void *find(void *begin, void const *end, basic_xref const &xref);
void *find(void *begin, void const *end, void const *from, void const *to);
}