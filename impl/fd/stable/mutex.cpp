module;

#include <cstdlib>

#include <Windows.h>

module fd.mutex;

struct mutex_data : CRITICAL_SECTION
{
};

mutex_data* basic_mutex::data() const
{
    return data_.get();
}

basic_mutex::basic_mutex()
    : data_(new mutex_data)
{
    constexpr DWORD flags = RTL_CRITICAL_SECTION_FLAG_DYNAMIC_SPIN |
#ifdef _DEBUG
                            RTL_CRITICAL_SECTION_FLAG_FORCE_DEBUG_INFO
#else
                            RTL_CRITICAL_SECTION_FLAG_NO_DEBUG_INFO
#endif
        ;
    InitializeCriticalSectionEx(data_.get(), 0, flags);
}

basic_mutex::~basic_mutex()
{
    DeleteCriticalSection(data_.get());
}

void basic_mutex::lock() noexcept
{
    EnterCriticalSection(data_.get());
}

void basic_mutex::unlock() noexcept
{
    LeaveCriticalSection(data_.get());
}

//----

void mutex::lock() noexcept
{
    basic_mutex::lock();
    if (data()->RecursionCount > 1)
        std::abort();
}