module;

#include <cheat/tools/interface.h>

#include <nstd/runtime_assert_core.h>

#include <future>
#include <functional>

export module cheat.hooks.loader;
export import cheat.hooks.initializer;
import cheat.hooks.base;

struct simple_info
{
    std::function<cheat::hooks::base* ()> getter;
    bool(*initialized)();
};

void add_pending(simple_info&& hook) runtime_assert_noexcept;

export namespace cheat::hooks
{
    template<class HookInterface>
    void add( ) runtime_assert_noexcept
    {
        using inst = one_instance<HookInterface*>;

        simple_info info;
        info.getter = []( )->base*
        {
            return inst::get_ptr( );
        };
        info.initialized = inst::initialized;

        add_pending(std::move(info));
    }

    std::future<bool> start( ) runtime_assert_noexcept;
    void stop( ) runtime_assert_noexcept;
}