#pragma once

#include "interface.h"

namespace fd
{
struct prepared_hook_data
{
    void *target;
    void *replace;
    void **original;
};

struct basic_hook_backend : basic_interface
{
    virtual void *create(void *target, void *replace) = 0;

    virtual void create(prepared_hook_data const &data)
    {
        *data.original = create(data.target, data.replace);
    }

    virtual void enable()  = 0;
    virtual void disable() = 0;

    virtual void enable(void *target)  = 0;
    virtual void disable(void *target) = 0;
};

} // namespace fd