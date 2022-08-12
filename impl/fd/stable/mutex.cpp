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
    InitializeCriticalSection(data_.get());
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
    if (data()->RecursionCount >= 1)
        std::abort();
    basic_mutex::lock();
}
