// ReSharper disable CppMemberFunctionMayBeStatic
#include "minhook.h"
#include "prepared_data.h"
#include "diagnostics/hook_error.h"

#include <MinHook.h>

#undef MH_ALL_HOOKS
#define MH_ALL_HOOKS nullptr // intellisense stfu

namespace fd
{
static char const* to_string(MH_STATUS const status)
{
    auto msg = MH_StatusToString(status);
    if (msg[2] == '_' /*MH_*/)
        msg += 3;
    return msg;
}

class minhook_error final : public hook_error
{
    MH_STATUS status_;

  public:
    minhook_error(MH_STATUS const status, char const* message)
        : hook_error(
              message
#ifdef _DEBUG
                  ? message
                  : to_string(status)
#endif
                  )
        , status_(status)
    {
    }

    char const* status() const override
    {
        return to_string(status_);
    }
};

static void process_status(MH_STATUS const status, char const* message = nullptr)
{
    if (status != MH_OK)
        throw minhook_error(status, message);
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

void hook_backend_minhook::create(prepared_hook_data const* data)
{
    process_status(MH_CreateHook(data->target, data->replace, data->original));
}

void hook_backend_minhook::enable()
{
    process_status(MH_EnableHook(MH_ALL_HOOKS));
}

void hook_backend_minhook::disable()
{
    process_status(MH_DisableHook(MH_ALL_HOOKS));
}

void hook_backend_minhook::enable(void* target)
{
    process_status(MH_EnableHook(target));
}

void hook_backend_minhook::disable(void* target)
{
    process_status(MH_DisableHook(target));
}
}