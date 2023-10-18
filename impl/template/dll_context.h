#pragma once
#include "noncopyable.h"

#include <Windows.h>

#include <cassert>
#include <cstdlib>

#define FD_DLLMAIN(...) BOOL WINAPI __VA_ARGS__(HINSTANCE handle, DWORD const reason, LPCVOID const reserved)

FD_DLLMAIN(DllMain);

namespace fd
{
static bool run_context();

static class : public noncopyable
{
    friend FD_DLLMAIN(::DllMain);

    HINSTANCE self_handle_;

    HANDLE thread_;
    DWORD thread_id_;

    bool start(HINSTANCE handle)
    {
        self_handle_ = handle;
        thread_      = CreateThread(
            nullptr, 0,
            [](LPVOID this_ptr) -> DWORD {
                auto const success = run_context();
                static_cast<decltype(this)>(this_ptr)->stop(success);
            },
            this, 0, &thread_id_);
        return thread_ != nullptr;
    }

    [[noreturn]]
    void stop(bool const success) const
    {
        assert(GetCurrentThreadId() == thread_id_);
        FreeLibraryAndExitThread(self_handle_, success ? EXIT_SUCCESS : EXIT_FAILURE);
    }

  public:
    [[maybe_unused]]
    bool pause() const
    {
        return SuspendThread(thread_) != static_cast<DWORD>(-1);
    }

    [[maybe_unused]]
    bool resume() const
    {
        return ResumeThread(thread_) != static_cast<DWORD>(-1);
    }

} context_holder;
} // namespace fd

// ReSharper disable once CppNonInlineFunctionDefinitionInHeaderFile
FD_DLLMAIN(DllMain)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH: {
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        if (!fd::context_holder.start(handle))
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