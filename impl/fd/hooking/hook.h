#pragma once
#include "fd/core.h"

namespace fd
{
using hook_name = _const<char*>;

struct hook_id
{
    void *target;

#ifdef _DEBUG
    hook_name name;
#else
    static constexpr hook_name name = "Unnamed";
#endif

    hook_id(nullptr_t)
        : hook_id(nullptr, "Unnamed")
    {
    }

    hook_id(void *target, hook_name name)
        : target(target)
#ifdef _DEBUG
        , name(name)
#endif
    {
        (void)name;
    }

    explicit operator bool() const
    {
        return target != nullptr;
    }
};

bool create_hook_data();
bool destroy_hook_data();

hook_id create_hook(void *target, void *replace, hook_name name, void **trampoline);
bool enable_hook(hook_id id);
bool enable_hook_lazy(hook_id id);
bool disable_hook(hook_id id);
bool disable_hook_lazy(hook_id id);
bool apply_lazy_hooks();
bool enable_hooks();
bool disable_hooks();
} // namespace fd