﻿// ReSharper disable CppMemberFunctionMayBeStatic
#include "hook/backend/minhook.h"
#include "hook/prepared_data.h"

#include <MinHook.h>

#include <cassert>

#undef MH_ALL_HOOKS
#define MH_ALL_HOOKS nullptr // intellisense stfu

namespace fd
{
[[maybe_unused]]
static char const* to_string(MH_STATUS const status)
{
    auto msg = MH_StatusToString(status);
    if (msg[2] == '_' /*MH_*/)
        msg += 3;
    return msg;
}

static bool process_status(MH_STATUS const status, [[maybe_unused]] char const* message = nullptr)
{
    assert(status == MH_OK);
    return status == MH_OK;
}

hook_backend_minhook::~hook_backend_minhook()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

hook_backend_minhook::hook_backend_minhook()
{
    process_status(MH_Initialize());
}

void* hook_backend_minhook::create(void* target, void* replace)
{
    void* original;
    process_status(MH_CreateHook(target, replace, &original));
    return original;
}

bool hook_backend_minhook::enable()
{
    return process_status(MH_EnableHook(MH_ALL_HOOKS));
}

bool hook_backend_minhook::disable()
{
    return process_status(MH_DisableHook(MH_ALL_HOOKS));
}

bool hook_backend_minhook::enable(void* target)
{
    return process_status(MH_EnableHook(target));
}

bool hook_backend_minhook::disable(void* target)
{
    return process_status(MH_DisableHook(target));
}
}