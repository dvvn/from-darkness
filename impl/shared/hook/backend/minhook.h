#pragma once
#include "noncopyable.h"
#include "prepared_data.h"

namespace fd
{
struct hook_backend_minhook final : noncopyable
{
    ~hook_backend_minhook();
    hook_backend_minhook();

    void* create(void* target, void* replace);
    void create(prepared_hook_data const* data);

    void enable();
    void disable();

    void enable(void* target);
    void disable(void* target);
};
} // namespace fd