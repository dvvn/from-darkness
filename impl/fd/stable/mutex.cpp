module;

#include <Windows.h>

module fd.mutex;

struct mutex_data : CRITICAL_SECTION
{
};

mutex::mutex()
    : data_(new mutex_data)
{
    InitializeCriticalSection(data_.get());
}

mutex::~mutex()
{
    DeleteCriticalSection(data_.get());
}

void mutex::lock() noexcept
{
    EnterCriticalSection(data_.get());
}

void mutex::unlock() noexcept
{
    LeaveCriticalSection(data_.get());
}
