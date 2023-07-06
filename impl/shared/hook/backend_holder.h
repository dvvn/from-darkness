#pragma once

#include "basic_backend.h"

#ifdef FD_ALLOCATE_HOOK_BACKEND
#include <memory>
#endif

namespace fd
{
#ifdef FD_ALLOCATE_HOOK_BACKEND
struct hook_backend_holder : std::unique_ptr<basic_hook_backend>
{
    hook_backend_holder(basic_hook_backend *b)
        : std::unique_ptr<basic_hook_backend>(b)
    {
    }

    operator basic_hook_backend *() const
    {
        return get();
    }
};
#else
using hook_backend_holder = basic_hook_backend *;
#endif

hook_backend_holder hook_backend();

} // namespace fd