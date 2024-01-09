#pragma once

#include "core/basic_context.h"

namespace fd
{
namespace detail
{
inline bool attach_thread(HINSTANCE instance);
}

namespace win
{
inline DWORD make_exit_code(bool const val) noexcept
{
    return val ? EXIT_SUCCESS : EXIT_FAILURE;
}
} // namespace win

class basic_dll_context
{
    friend bool detail::attach_thread(HINSTANCE);

    HINSTANCE instance_;

    struct
    {
        HANDLE handle;
        DWORD id;
    } thread_;

    /*DECLSPEC_NORETURN*/ void stop(DWORD const exit_code) const
    {
        assert(thread_.id == GetCurrentThreadId());
        FreeLibraryAndExitThread(instance_, exit_code);
    }

    void attach(HINSTANCE instance, HANDLE thread_handle = GetCurrentThread(), DWORD const thread_id = GetCurrentThreadId()) noexcept
    {
        instance_      = instance;
        thread_.handle = thread_handle;
        thread_.id     = thread_id;
    }

  public:
    basic_dll_context()
    {
        std::ignore = this;
    }

    bool pause()
    {
        return SuspendThread(thread_.handle) != static_cast<DWORD>(-1);
    }

    bool resume()
    {
        return ResumeThread(thread_.handle) != static_cast<DWORD>(-1);
    }

    bool terminate(DWORD const exit_code = EXIT_SUCCESS)
    {
        return TerminateThread(thread_.handle, exit_code);
    }
};

inline basic_dll_context* get_dll_context();
inline size_t get_dll_context_stack_size();

namespace detail
{
bool attach_thread(HINSTANCE instance)
{
    auto const ctx = get_dll_context();
    DWORD thread_id;
    auto const thread_handle = CreateThread(
        nullptr, get_dll_context_stack_size(),
        [](LPVOID raw_ctx) -> DWORD {
            // ReSharper disable once CppDeclarationHidesUncapturedLocal
            auto const ctx           = static_cast<basic_dll_context*>(raw_ctx);
            auto const attach_result = attach_context();
            auto const exit_code     = win::make_exit_code(attach_result);
            ctx->stop(exit_code);
            return exit_code;
        },
        ctx, CREATE_SUSPENDED, &thread_id);
    if (thread_handle != INVALID_HANDLE_VALUE)
    {
        ctx->attach(instance, thread_handle, thread_id);
        if (ctx->resume())
            return true;
        ctx->terminate(EXIT_FAILURE);
    }
    return false;
}

inline BOOL dll_main(HINSTANCE instance, DWORD const reason, LPCVOID const reserved) noexcept
{
    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        if (!attach_thread(instance))
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
} // namespace detail
} // namespace fd

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
BOOL WINAPI DllMain(HINSTANCE instance, DWORD const reason, LPCVOID const reserved)
{
    return fd::detail::dll_main(instance, reason, reserved);
}