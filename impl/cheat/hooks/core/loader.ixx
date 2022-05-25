module;

#include <cheat/core/object.h>

#include <array>
#include <future>

export module cheat.hooks.loader;
import cheat.hooks.base;

using hook_base = cheat::hooks::base;

// // count added because std::is_constructible/destructible always true on any index
// // must by set in makefile
// #ifndef CHEAT_HOOKS_COUNT
// #define CHEAT_HOOKS_COUNT 1024 // huge value to break build
// #endif

struct basic_hooks_loader
{
    virtual ~basic_hooks_loader() = default;

    virtual std::future<bool> start() = 0;
    virtual void stop() = 0;

    template <size_t... I>
    void fill(const std::index_sequence<I...> = {})
    {
        (add(&CHEAT_OBJECT_GET(hook_base, I)), ...);
    }

  protected:
    virtual void add(hook_base* const hook) = 0;
};

CHEAT_OBJECT(loader, basic_hooks_loader);

export namespace cheat::hooks
{
    using ::loader;
}
