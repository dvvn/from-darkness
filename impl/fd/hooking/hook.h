#pragma once

#include <boost/predef.h>

#ifndef off
#define _OFF_DEFINED
#define off 1337
#endif

namespace fd
{
#if FD_DEFAULT_LOG_LEVEL == off
using hook_id = uintptr_t;
#else
#if BOOST_ARCH_X86 == BOOST_VERSION_NUMBER_AVAILABLE
using hook_id = uint64_t;
#elif BOOST_ARCH_X86_64 == BOOST_VERSION_NUMBER_AVAILABLE

#endif
#endif
using hook_name = char const *;

hook_id create_hook(void *target, void *replace, hook_name name, void **trampoline);
bool enable_hook(hook_id id);
bool disable_hook(hook_id id);
bool enable_hooks();
bool disable_hooks();
} // namespace fd

#ifdef _OFF_DEFINED
#undef _OFF_DEFINED
#undef off
#endif