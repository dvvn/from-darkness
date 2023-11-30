#pragma once
#include "tier1/noncopyable.h"

namespace FD_TIER(2)
{
struct hook_backend_minhook final : noncopyable
{
    ~hook_backend_minhook();
    hook_backend_minhook();

    void* create(void* target, void* replace);

    bool enable();
    bool disable();

    bool enable(void* target);
    bool disable(void* target);
};
} // namespace FD_TIER(2)