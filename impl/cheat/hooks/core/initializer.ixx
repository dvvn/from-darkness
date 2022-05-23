module;

#include <cheat/core/object.h>

export module cheat.hooks.initializer;

struct hooks_initializer
{
    virtual ~hooks_initializer() = default;
    virtual void operator()() = 0;
};

constexpr size_t _Basic_hooks_init = 1;
constexpr size_t _Csgo_hooks_init = 2;

void init_hooks();

export namespace cheat::hooks
{
    using ::init_hooks;
}
