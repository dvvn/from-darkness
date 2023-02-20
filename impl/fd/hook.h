#pragma once

#include <fd/string.h>

namespace fd
{
struct basic_hook
{
    virtual ~basic_hook() = default;

    virtual bool enable()  = 0;
    virtual bool disable() = 0;

    virtual string_view name() const
    {
        return "DEFAULT";
    }

    virtual bool initialized() const = 0;
    virtual bool active() const      = 0;
};
} // namespace fd