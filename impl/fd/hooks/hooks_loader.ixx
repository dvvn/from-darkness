module;

#include <fd/object.h>

#include <algorithm>
#include <array>

export module fd.hooks_loader;
import fd.hook_base;

using hook_base = fd::hook_base;

struct basic_hooks_loader
{
    virtual ~basic_hooks_loader() = default;

    virtual void disable() = 0;
    virtual void unload()  = 0;

    virtual bool init(const bool stop_on_error = true) = 0;
    virtual void store(hook_base* const hook)   = 0;
};

FD_OBJECT(hooks_loader, basic_hooks_loader);

export namespace fd
{
    using ::hooks_loader;
}
