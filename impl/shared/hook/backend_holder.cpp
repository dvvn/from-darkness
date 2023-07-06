#include "backend_holder.h"

#if __has_include(<MinHook.h>)
#include "backend/minhook.h"
#define HOOK_BACKEND hook_backend_minhook
#else

#endif

namespace fd
{
hook_backend_holder hook_backend()
{
    return HOOK_BACKEND();
}
}