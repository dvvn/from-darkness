module;

#include <fd/object.h>

#include <algorithm>
#include <array>

export module fd.hooks_loader;
import fd.hook_base;

using hook_base = fd::hook_base;

template <class T>
hook_base* _Get_hook_base(T* ptr)
{
    return reinterpret_cast<hook_base*>(ptr);
}

struct basic_hooks_loader
{
    virtual ~basic_hooks_loader() = default;

    template <class... C>
    bool load(const bool stop_on_error = true)
    {
        (store(_Get_hook_base(&FD_OBJECT_GET(C*))), ...);
        return init(stop_on_error);
    }

    virtual void disable() = 0;
    virtual void unload()  = 0;

  protected:
    virtual bool init(const bool stop_on_error) = 0;
    virtual void store(hook_base* const hook)   = 0;
};

FD_OBJECT(hooks_loader, basic_hooks_loader);

export namespace fd
{
    using ::hooks_loader;
}
