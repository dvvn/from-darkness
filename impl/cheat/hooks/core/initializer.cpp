module;

#include <cheat/core/object.h>

#include <concepts>

module cheat.hooks.initializer;

template <size_t I>
static auto& _Get_hook()
{
    return *CHEAT_OBJECT_GET(hooks_initializer, I);
}

template <size_t I>
concept can_init = requires
{
    _Get_hook<I>();
};

template <size_t I>
static void _Try_init()
{
    if constexpr (can_init<I>)
        std::invoke(_Get_hook<I>());
}

template <size_t... N>
static void _Init()
{
    (_Try_init<N>(), ...);
}

void init_hooks()
{
    _Init<_Basic_hooks_init, _Csgo_hooks_init>();
}
