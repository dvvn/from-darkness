module;

#include <cheat/hooks/interface.h>

export module cheat.hooks.directx.reset;

export namespace cheat::hooks::directx
{
    CHEAT_HOOK(reset, 1);
}
