#include <fd/assert.h>
#include <fd/exception.h>
#include <fd/thread_pool_impl.h>

#include <optional>

namespace fd
{
#if 0
//unused
template <template <typename...> class T, typename... Args>
class safe_storage
{
    using storage_type = T<Args...>;
    using mutex_type   = mutex;

    storage_type storage_;
    mutex_type mtx_;

    class getter : public lock_guard<mutex_type>
    {
        storage_type* s_;

      public:
        getter(safe_storage* ptr)
            : lock_guard<mutex_type>(ptr->mtx_)
        {
            s_ = ptr->storage_;
        }

        storage_type* operator->() const
        {
            return s_;
        }
    };

  public:
    storage_type* get_unsafe()
    {
        return &storage_;
    }

    getter get()
    {
        return this;
    }

    getter operator->()
    {
        return this;
    }
};
#endif

basic_thread_data::basic_thread_data(const HANDLE handle)
    : handle_(handle)
{
}

basic_thread_data::operator bool() const
{
    return handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE;
}

// ReSharper disable CppMemberFunctionMayBeConst
bool basic_thread_data::pause()
{
    return SuspendThread(handle_) != 0u;
}

bool basic_thread_data::resume()
{
    return ResumeThread(handle_) != 0u;
}

void basic_thread_data::terminate()
{
    if (!*this)
        return;

    TerminateThread(handle_, EXIT_SUCCESS);
    CloseHandle(handle_);
}

// ReSharper restore CppMemberFunctionMayBeConst

//---

thread_data::~thread_data()
{
    this->terminate();
}

// ReSharper disable once CppPossiblyUninitializedMember
thread_data::thread_data(void* fn, void* fnParams, const bool suspend)
    : basic_thread_data(CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(fn), fnParams, suspend ? CREATE_SUSPENDED : 0, &id_))
{
}

thread_data::thread_data(thread_data&& other) noexcept
    : basic_thread_data(other)
    , id_(other.id_)
{
    static_cast<basic_thread_data&>(other) = INVALID_HANDLE_VALUE;
}

thread_data& thread_data::operator=(thread_data&& other) noexcept
{
    using std::swap;
    swap<basic_thread_data>(*this, other);
    swap(id_, other.id_);
    return *this;
}

bool thread_data::operator==(const DWORD id) const
{
    return this->id_ == id;
}

//---

using optional_mtx_lock = std::optional<std::lock_guard<std::mutex>>;

bool thread_pool::worker_impl()
{
#ifdef _DEBUG
    optional_mtx_lock guard(threadsMtx_);
#else
    threadsMtx_.lock();
#endif
    // range version doesnt work
    basic_thread_data thisThread(*std::find(threads_.begin(), threads_.end(), GetCurrentThreadId()));
#ifdef _DEBUG
    guard.reset();
#else
    threadsMtx_.unlock();
#endif

    for (;;)
    {
        function_type task;
        while (funcs_.try_pop(task))
        {
            try
            {
                task();
            }
            catch (...)
            {
                // todo: any external exception -> false
                // special 'interrupt' exception -> true
                return FALSE;
            }
        }

        if (!thisThread.pause())
        {
            unload();
            return FALSE;
        }
    }

    // ReSharper disable once CppUnreachableCode
    return TRUE;
}

bool thread_pool::store_func(function_type&& func, const bool resumeThreads) noexcept
{
    const auto canSpawnThread = [&] {
        for (auto& t : threads_)
        {
            if (t.resume())
                return true;
        }
        return false;
    };

    optional_mtx_lock guard(threadsMtx_);
    auto              resetGuard = true;

    if (!resumeThreads)
    {
        if (threads_.empty())
            return false;
    }
    else if (funcs_.was_size() >= threads_.size() && !canSpawnThread())
    {
        // spawn new thread
        thread_data data(reinterpret_cast<void*>(worker), this, false);
        if (!data)
            return false;
        threads_.emplace_back(std::move(data));
        // wait until thread find itself from the loop
        resetGuard = false;
    }

    if (resetGuard)
        guard.reset();

    return funcs_.try_push(std::move(func));
}

thread_pool::thread_pool()
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    FD_ASSERT(info.dwNumberOfProcessors > 0);
    threads_.reserve(info.dwNumberOfProcessors);
}

thread_pool::~thread_pool()
{
    if (!funcs_.was_empty())
        this->wait();

#ifdef _DEBUG
    threads_.clear();
#endif
}

void thread_pool::wait()
{
    //"simple" version of task
    std::binary_semaphore sem(1);
    const auto            stored = this->store_func([&] {
        sem.release();
    });
    if (stored)
        sem.acquire();
}

bool thread_pool::add_simple(function_type&& func)
{
    return this->store_func(std::move(func));
}

auto thread_pool::add(function_type&& func) -> task_type
{
    task_type  t(new lockable_task(std::move(func), true));
    const auto stored = this->store_func([=] {
        t->start();
    });
    if (!stored)
        return task_type(new finished_task());
    return t;
}

auto thread_pool::add_lazy(function_type&& func) -> task_type
{
    auto newFunc = [this, fn = std::move(func)](auto& sem) mutable {
        const auto stored = this->store_func([&] {
            fn();
            sem.release();
        });
        if (!stored)
            sem.release();
    };

    return task_type(new lockable_task(std::move(newFunc), true));
}
}