module;

#include <cheat/hooks/interface.h>

export module cheat.hooks.directx.present;

export namespace cheat::hooks::directx
{
    CHEAT_HOOK(present, 2);
}
