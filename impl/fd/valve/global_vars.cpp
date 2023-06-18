#include "global_vars.h"

namespace fd::valve
{
static_assert(sizeof(IGlobalVarsBase) == 0x40);
static_assert(sizeof(IGlobalVars) == 0x64);
} // namespace fd::valve
