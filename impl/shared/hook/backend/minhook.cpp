#include "minhook.h"
#include "noncopyable.h"
#include "diagnostics/hook_error.h"

#include <MinHook.h>

#undef MH_ALL_HOOKS
#define MH_ALL_HOOKS nullptr // intellisense stfu

namespace fd
{
class minhook_error final : public hook_error
{
    MH_STATUS status_;

  public:
    minhook_error(MH_STATUS status, char const *message)
        : hook_error(
              message
#ifdef _DEBUG
                  ? message
                  : default_exception_message
#endif
              )
        , status_(status)
    {
    }

    char const *status() const override
    {
        auto msg = MH_StatusToString(status_);
        if (msg[2] == '_' /*MH_*/)
            msg += 3;
        return msg;
    }

    static void create(MH_STATUS status, char const *message = nullptr)
    {
        if (status != MH_OK)
            throw minhook_error(status, message);
    }
};

struct backend_minhook final : basic_hook_backend, noncopyable
{
#ifdef FD_ALLOCATE_HOOK_BACKEND
    ~backend_minhook() override
#else
    ~backend_minhook()
#endif
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    backend_minhook()
    {
        minhook_error::create(MH_Initialize());
    }

    void *create(void *target, void *replace) override
    {
        void *original;
        minhook_error::create(MH_CreateHook(target, replace, &original));
        return original;
    }

    void create(prepared_hook_data const &data) override
    {
        minhook_error::create(MH_CreateHook(data.target, data.replace, data.original));
    }

    void enable() override
    {
        minhook_error::create(MH_EnableHook(MH_ALL_HOOKS));
    }

    void disable() override
    {
        minhook_error::create(MH_DisableHook(MH_ALL_HOOKS));
    }

    void enable(void *target) override
    {
        minhook_error::create(MH_EnableHook(target));
    }

    void disable(void *target) override
    {
        minhook_error::create(MH_DisableHook(target));
    }
};

basic_hook_backend *hook_backend_minhook()
{
#ifdef FD_ALLOCATE_HOOK_BACKEND
    return new backend_minhook();
#else
    static backend_minhook b;
    return &b;
#endif
}
}