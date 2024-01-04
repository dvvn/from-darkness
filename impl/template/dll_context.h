#pragma once
#include "basic_context.h"

#include <Windows.h>

#include <cassert>
#include <cstdlib>
#include <tuple>

namespace fd
{
namespace detail
{
class dll_context_holder : public basic_context
{
    HINSTANCE instance_;
    HANDLE thread_;
    DWORD thread_id_;

    DECLSPEC_NORETURN void stop(bool const success) const
    {
        assert(GetCurrentThreadId() == thread_id_);
        FreeLibraryAndExitThread(instance_, success ? EXIT_SUCCESS : EXIT_FAILURE);
    }

  protected:
    bool attach(HINSTANCE instance);

  public:
    dll_context_holder()
    {
        std::ignore = this;
    }

    bool pause()
    {
        return SuspendThread(thread_) != static_cast<DWORD>(-1);
    }

    bool resume()
    {
        return ResumeThread(thread_) != static_cast<DWORD>(-1);
    }
};

inline struct : dll_context_holder
{
    BOOL operator()(HINSTANCE instance, DWORD const reason, LPCVOID const reserved) noexcept
    {
        // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
        switch (reason)
        {
        case DLL_PROCESS_ATTACH: {
            // Initialize once for each new process.
            // Return FALSE to fail DLL load.
            if (!this->attach(instance))
                return FALSE;
            break;
        }
#if 0
    case DLL_THREAD_ATTACH: // Do thread-specific initialization.
        break;
    case DLL_THREAD_DETACH: // Do thread-specific cleanup.
        break;
#endif
        case DLL_PROCESS_DETACH:
            if (reserved != nullptr) // do not do cleanup if process termination scenario
            {
                break;
            }

            // Perform any necessary cleanup.
            break;
        }

        return TRUE;
    }
} dll_context;
} // namespace detail

using context = detail::dll_context_holder;

bool context_holder(context *ctx);

inline bool context::attach(HINSTANCE instance)
{
    instance_ = instance;
    thread_   = CreateThread(
        nullptr, 0,
        [](LPVOID this_ptr) -> DWORD {
            auto const ctx     = static_cast<dll_context_holder *>(this_ptr);
            auto const success = context_holder(ctx);
            ctx->stop(success);
        },
        this, 0, &thread_id_);
    return thread_ != INVALID_HANDLE_VALUE;
}
} // namespace fd

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
BOOL WINAPI DllMain(HINSTANCE instance, DWORD const reason, LPCVOID const reserved)
{
    return fd::detail::dll_context(instance, reason, reserved);
}