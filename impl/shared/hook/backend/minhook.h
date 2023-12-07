#pragma once
#include "noncopyable.h"

#include <MinHook.h>

#include <cassert>

inline char const* to_string(MH_STATUS const status)
{
    auto msg = MH_StatusToString(status);
    if (msg[2] == '_' /*MH_*/)
        msg += 3;
    return msg;
}

// ReSharper disable CppMemberFunctionMayBeStatic
// ReSharper disable CppZeroConstantCanBeReplacedWithNullptr

namespace fd
{
struct hook_backend_minhook final : noncopyable
{
    ~hook_backend_minhook()
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    hook_backend_minhook()
    {
        auto const status = MH_Initialize();
        assert(status == MH_OK);
    }

    void* create(void* target, void* replace)
    {
        void* original    = nullptr;
        auto const status = MH_CreateHook(target, replace, &original);
        assert(status == MH_OK);
        return original;
    }

    bool enable()
    {
        auto const status = MH_EnableHook(MH_ALL_HOOKS);
        assert(status == MH_OK);
        return status == MH_OK;
    }

    bool disable()
    {
        auto const status = MH_DisableHook(MH_ALL_HOOKS);
        assert(status == MH_OK);
        return status == MH_OK;
    }

    bool enable(void* target)
    {
        auto const status = MH_EnableHook(target);
        assert(status == MH_OK);
        return status == MH_OK;
    }

    bool disable(void* target)
    {
        auto const status = MH_DisableHook(target);
        assert(status == MH_OK);
        return status == MH_OK;
    }
};
} // namespace fd