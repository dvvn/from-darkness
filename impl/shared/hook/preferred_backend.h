#pragma once

#include "basic_backend.h"

#if __has_include(<MinHook.h>)
#include "backend/minhook.h"
#define HOOK_BACKEND backend_minhook
#else

#endif

namespace fd
{
using preferred_hook_backend = HOOK_BACKEND;
} // namespace fd

#undef HOOK_BACKEND