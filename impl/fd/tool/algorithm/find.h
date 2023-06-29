#pragma once

namespace fd
{
struct basic_pattern_segment;
struct basic_pattern;

void *find(void *begin, void *end, basic_pattern_segment const &segment);
void *find(void *begin, void *end, basic_pattern const &pattern);
}