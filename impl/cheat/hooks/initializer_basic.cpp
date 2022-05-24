module;

#include <cheat/core/object.h>

module cheat.hooks.initializer;
import cheat.hooks.loader;

import cheat.hooks.winapi.wndproc;
import cheat.hooks.directx.present;
import cheat.hooks.directx.reset;

struct initializer_basic final : hooks_initializer
{
    void operator()() override
    {
        using namespace cheat::hooks;
        loader->add<winapi::wndproc>();
        loader->add<directx::reset>();
        loader->add<directx::present>();
    }
};

CHEAT_OBJECT_BIND(hooks_initializer, _Basic_hooks_init, initializer_basic, _Basic_hooks_init)
