#pragma once

namespace fd
{
struct basic_hook_lazy;

struct basic_hook
{
    virtual ~basic_hook() = default;

    virtual bool enable()  = 0;
    virtual bool disable() = 0;

    virtual char const *name() const = 0;

    virtual basic_hook_lazy *lazy() = 0;
};

struct basic_hook_lazy
{
    virtual ~basic_hook_lazy() = default;

    virtual bool enable()  = 0;
    virtual bool disable() = 0;
};
} // namespace fd