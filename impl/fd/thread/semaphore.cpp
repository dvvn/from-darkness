module;

#include <Windows.h>

#include <utility>

module fd.semaphore;

semaphore::~semaphore()
{
    if (handle_)
        CloseHandle(handle_);
}

semaphore::semaphore()
{
    handle_ = nullptr;
}

semaphore::semaphore(const size_type init_count, const size_type max_count)
{
    handle_ = CreateSemaphore(nullptr, init_count, max_count, nullptr);
}

semaphore::semaphore(semaphore&& other)
{
    handle_ = std::exchange(other.handle_, nullptr);
}

semaphore& semaphore::operator=(semaphore&& other)
{
    using std::swap;
    swap(handle_, other.handle_);
    return *this;
}

void semaphore::release(const size_type count)
{
    ReleaseSemaphore(handle_, count, nullptr);
}

void semaphore::acquire()
{
    WaitForSingleObject(handle_, INFINITE);
}
