#include "hook/backend/minhook.h"

#include <MinHook.h>

#include <cassert>

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr

[[maybe_unused]]
static char const* to_string(MH_STATUS const status)
{
    auto msg = MH_StatusToString(status);
    if (msg[2] == '_' /*MH_*/)
        msg += 3;
    return msg;
}

namespace fd
{
hook_backend_minhook::~hook_backend_minhook()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

hook_backend_minhook::hook_backend_minhook()
{
    auto const status = MH_Initialize();
    assert(status == MH_OK);
}

void* hook_backend_minhook::create(void* target, void* replace)
{
    void* original    = nullptr;
    auto const status = MH_CreateHook(target, replace, &original);
    assert(status == MH_OK);
    return original;
}

bool hook_backend_minhook::enable()
{
    auto const status = MH_EnableHook(MH_ALL_HOOKS);
    assert(status == MH_OK);
    return status == MH_OK;
}

bool hook_backend_minhook::disable()
{
    auto const status = MH_DisableHook(MH_ALL_HOOKS);
    assert(status == MH_OK);
    return status == MH_OK;
}

bool hook_backend_minhook::enable(void* target)
{
    auto const status = MH_EnableHook(target);
    assert(status == MH_OK);
    return status == MH_OK;
}

bool hook_backend_minhook::disable(void* target)
{
    auto const status = MH_DisableHook(target);
    assert(status == MH_OK);
    return status == MH_OK;
}
} // namespace fd
