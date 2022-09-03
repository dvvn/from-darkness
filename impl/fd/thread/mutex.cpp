module;

#include <cstdlib>

#include <Windows.h>

module fd.mutex;

static constexpr DWORD _Crt_section_init_flags = RTL_CRITICAL_SECTION_FLAG_DYNAMIC_SPIN |
#ifdef _DEBUG
                                                 RTL_CRITICAL_SECTION_FLAG_FORCE_DEBUG_INFO
#else
                                                 RTL_CRITICAL_SECTION_FLAG_NO_DEBUG_INFO
#endif
    ;

mutex::mutex()
{
    InitializeCriticalSectionEx(&sec_, 0, _Crt_section_init_flags);
}

mutex::~mutex()
{
    DeleteCriticalSection(&sec_);
}

void mutex::lock() noexcept
{
    EnterCriticalSection(&sec_);
    if (sec_.RecursionCount > 1)
        std::abort();
}

void mutex::unlock() noexcept
{
    LeaveCriticalSection(&sec_);
}

//----

recursive_mutex::recursive_mutex()
{
    InitializeCriticalSectionEx(&sec_, 0, _Crt_section_init_flags);
}

recursive_mutex::~recursive_mutex()
{
    DeleteCriticalSection(&sec_);
}

void recursive_mutex::lock() noexcept
{
    EnterCriticalSection(&sec_);
}

void recursive_mutex::unlock() noexcept
{
    LeaveCriticalSection(&sec_);
}
