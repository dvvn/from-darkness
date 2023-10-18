#pragma once
#include "noncopyable.h"

namespace fd
{
struct hook_backend_minhook final : noncopyable
{
    ~hook_backend_minhook();
    hook_backend_minhook();

    void* create(void* target, void* replace);

    void enable();
    void disable();

    void enable(void* target);
    void disable(void* target);
};
} // namespace fd