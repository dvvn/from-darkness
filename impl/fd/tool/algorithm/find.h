#pragma once

#include "basic_pattern.h"

namespace fd
{
void *find_pattern(void *begin, void *end, basic_pattern const *pat);
}
