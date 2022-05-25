module;

#include <cheat/hooks/interface.h>

export module cheat.hooks.winapi.wndproc;

namespace cheat::hooks::winapi
{
    CHEAT_HOOK(wndproc, 0);
}
