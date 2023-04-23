#pragma once

namespace fd
{
struct basic_hook
{
    virtual ~basic_hook() = default;

    virtual bool enable()  = 0;
    virtual bool disable() = 0;

    virtual char const *name() const
    {
        return "Unknown";
    }

    virtual bool initialized() const = 0;
    virtual bool active() const      = 0;
};
} // namespace fd