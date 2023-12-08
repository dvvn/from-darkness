#include "pattern/make.h"
#include "pattern/to_string.h"

namespace fd
{
static_assert(to_string("01 ? 02 ? AF FF 0C"_pat) == "01 ? 02 ? AF FF 0C");
static_assert(to_string("1 2 3 4 ?? ? ? ff FF af 04"_pat) == "01 02 03 04 ? ? ? FF FF AF 04");
} // namespace fd
