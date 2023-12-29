#pragma once
#include "basic_context.h"

#include <Windows.h>

#include <cassert>
#include <cstdlib>

namespace fd
{
namespace detail
{
class dll_context : public basic_context
{
    HINSTANCE instance_;

    HANDLE thread_;
    DWORD thread_id_;

    DECLSPEC_NORETURN void stop(bool const success) const
    {
        assert(GetCurrentThreadId() == thread_id_);
        FreeLibraryAndExitThread(instance_, success ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    bool run();

  public:
    // ReSharper disable once CppPossiblyUninitializedMember
    dll_context()
    {
    }

    bool pause() const
    {
        return SuspendThread(thread_) != static_cast<DWORD>(-1);
    }

    bool resume() const
    {
        return ResumeThread(thread_) != static_cast<DWORD>(-1);
    }

    bool start(HINSTANCE instance)
    {
        instance_ = instance;
        thread_   = CreateThread(
            nullptr, 0,
            [](LPVOID this_ptr) -> DWORD {
                auto const ctx     = static_cast<dll_context *>(this_ptr);
                auto const success = ctx->run();
                ctx->stop(success);
            },
            this, 0, &thread_id_);
        return thread_ != INVALID_HANDLE_VALUE;
    }
};

inline dll_context dll_context_instance;
} // namespace detail

using context = detail::dll_context;
} // namespace fd

// ReSharper disable once CppInconsistentNaming
// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
BOOL WINAPI DllMain(HINSTANCE instance, DWORD const reason, LPCVOID const reserved)
{
    using fd::detail::dll_context_instance;

    // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        if (!dll_context_instance.start(instance))
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

#undef FD_DLLMAIN