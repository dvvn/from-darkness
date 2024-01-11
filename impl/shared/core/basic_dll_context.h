#pragma once

#include "core/basic_context.h"

namespace fd
{
namespace detail
{
bool attach_thread(HINSTANCE instance) noexcept;
}

class basic_dll_context : public basic_context
{
    friend bool detail::attach_thread(HINSTANCE) noexcept;

    HINSTANCE instance_;

    struct
    {
        HANDLE handle;
        DWORD id;
    } thread_;

    /*DECLSPEC_NORETURN*/ void stop(DWORD exit_code) const;

    void attach(HINSTANCE instance, HANDLE thread_handle = GetCurrentThread(), DWORD thread_id = GetCurrentThreadId()) noexcept;

  public:
    basic_dll_context();

    bool pause();
    bool resume();
    bool terminate(DWORD exit_code = EXIT_SUCCESS);
};

basic_dll_context* get_dll_context() noexcept;
size_t get_dll_context_stack_size() noexcept;
} // namespace fd
