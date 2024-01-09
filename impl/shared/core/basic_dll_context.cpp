#include "basic_dll_context.h"

namespace fd
{
void basic_dll_context::stop(DWORD const exit_code) const
{
    assert(thread_.id == GetCurrentThreadId());
    FreeLibraryAndExitThread(instance_, exit_code);
}

void basic_dll_context::attach(HINSTANCE instance, HANDLE thread_handle, DWORD const thread_id) noexcept
{
    instance_      = instance;
    thread_.handle = thread_handle;
    thread_.id     = thread_id;
}

basic_dll_context::basic_dll_context()
{
    std::ignore = this;
}

// ReSharper disable CppMemberFunctionMayBeConst

bool basic_dll_context::pause()
{
    return SuspendThread(thread_.handle) != static_cast<DWORD>(-1);
}

bool basic_dll_context::resume()
{
    return ResumeThread(thread_.handle) != static_cast<DWORD>(-1);
}

bool basic_dll_context::terminate(DWORD const exit_code)
{
    return TerminateThread(thread_.handle, exit_code);
}

// ReSharper restore CppMemberFunctionMayBeConst

namespace win
{
static DWORD make_exit_code(bool const val) noexcept
{
    return val ? EXIT_SUCCESS : EXIT_FAILURE;
}
} // namespace win

namespace detail
{
bool attach_thread(HINSTANCE instance) noexcept
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

static BOOL dll_main(HINSTANCE instance, DWORD const reason, LPCVOID const reserved) noexcept
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
BOOL WINAPI DllMain(HINSTANCE instance, DWORD const reason, LPCVOID const reserved)
{
    return fd::detail::dll_main(instance, reason, reserved);
}