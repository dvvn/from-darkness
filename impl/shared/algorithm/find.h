#pragma once

namespace fd
{
struct basic_pattern;
struct basic_xref;

void *find(void *begin, void *end, basic_pattern const &pattern);
void *find(void *begin, void *end, basic_xref const &xref);
void *find(void *begin, void *end, void const *from, void const *to);
}