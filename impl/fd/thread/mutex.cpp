module;

#include <cstdlib>

#include <Windows.h>

module fd.mutex;

constexpr auto aa = sizeof(CRITICAL_SECTION);

template <typename T>
static auto _Get_crt_section(T& data)
{
    return reinterpret_cast<CRITICAL_SECTION*>(&data);
}

static constexpr DWORD _Crt_section_init_flags = RTL_CRITICAL_SECTION_FLAG_DYNAMIC_SPIN |
#ifdef _DEBUG
                                                 RTL_CRITICAL_SECTION_FLAG_FORCE_DEBUG_INFO
#else
                                                 RTL_CRITICAL_SECTION_FLAG_NO_DEBUG_INFO
#endif
    ;

mutex::mutex()
{
    InitializeCriticalSectionEx(_Get_crt_section(buff_), 0, _Crt_section_init_flags);
}

mutex::~mutex()
{
    DeleteCriticalSection(_Get_crt_section(buff_));
}

void mutex::lock() noexcept
{
    auto data = _Get_crt_section(buff_);
    EnterCriticalSection(data);
    if (data->RecursionCount > 1)
        std::abort();
}

void mutex::unlock() noexcept
{
    LeaveCriticalSection(_Get_crt_section(buff_));
}

//----

recursive_mutex::recursive_mutex()
{
    InitializeCriticalSectionEx(_Get_crt_section(buff_), 0, _Crt_section_init_flags);
}

recursive_mutex::~recursive_mutex()
{
    DeleteCriticalSection(_Get_crt_section(buff_));
}

void recursive_mutex::lock() noexcept
{
    EnterCriticalSection(_Get_crt_section(buff_));
}

void recursive_mutex::unlock() noexcept
{
    LeaveCriticalSection(_Get_crt_section(buff_));
}
