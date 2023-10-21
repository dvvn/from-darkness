#pragma once
#include "noncopyable.h"

namespace fd
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
} // namespace fd