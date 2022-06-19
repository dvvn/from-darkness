module;

#include <fd/core/object.h>

export module fd.hooks_loader;
import fd.hook_base;

// // count added because std::is_constructible/destructible always true on any index
// // must by set in makefile
// #ifndef FD_HOOKS_COUNT
// #define FD_HOOKS_COUNT 1024 // huge value to break build
// #endif

struct basic_hooks_loader
{
    using hook_base = fd::hook_base;

    virtual ~basic_hooks_loader() = default;

    template <size_t... I>
    bool load(const std::index_sequence<I...> = {})
    {
        try
        {
            (store(&FD_OBJECT_GET(hook_base, I)), ...);
            load();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    virtual void disable_all() = 0;
    bool load_all();

    virtual void unload() = 0;

  protected:
    virtual void load()                       = 0;
    virtual void store(hook_base* const hook) = 0;
};

FD_OBJECT(hooks_loader, basic_hooks_loader);

export namespace fd
{
    using ::hooks_loader;
}
