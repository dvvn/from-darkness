module;

#include <fd/assert.h>

#include <Windows.h>

#include <algorithm>

module fd.thread_pool.impl;
import fd.task.impl;
import fd.functional.bind;

using namespace fd;

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

basic_thread_data::basic_thread_data(HANDLE handle)
    : handle_(handle)
{
}

basic_thread_data::operator bool() const
{
    return handle_ && handle_ != INVALID_HANDLE_VALUE;
}

bool basic_thread_data::pause()
{
    return SuspendThread(handle_);
}

bool basic_thread_data::resume()
{
    return ResumeThread(handle_);
}

void basic_thread_data::terminate()
{
    if (!*this)
        return;

    TerminateThread(handle_, EXIT_SUCCESS);
    CloseHandle(handle_);
}

//---

thread_data::~thread_data()
{
    this->terminate();
}

thread_data::thread_data(void* fn, void* fn_params, const bool suspend)
    : basic_thread_data(CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(fn), fn_params, suspend ? CREATE_SUSPENDED : 0, &id_))
{
}

thread_data::thread_data(thread_data&& other)
    : basic_thread_data(other)
{
    static_cast<basic_thread_data&>(other) = INVALID_HANDLE_VALUE;
    id_                                    = other.id_;
}

thread_data& thread_data::operator=(thread_data&& other)
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

bool thread_pool::worker_impl()
{
    basic_thread_data this_thread;

    const auto callback = [&]() -> uint8_t {
        if (funcs_.empty())
        {
            mtx_.unlock();
            if (!this_thread.pause())
                return 0;
        }
        else
        {
            auto task = std::move(funcs_.back());
            funcs_.pop_back();
            mtx_.unlock();

            try
            {
                invoke(task);
            }
            catch (...)
            {
                // todo: any external exception -> false
                // special 'interrupt' exception -> true
                return 1;
            }
        }

        return 2;
    };

    mtx_.lock();
    this_thread = *std::find(threads_.begin(), threads_.end(), GetCurrentThreadId());

    auto run = invoke(callback);
    while (run == 2)
    {
        mtx_.lock();
        run = invoke(callback);
    }

    if (!run)
        std::abort();

    return TRUE;
}

bool thread_pool::store_func(function_type&& func, const bool resume_threads) noexcept
{
    const lock_guard guard = mtx_;

    if (resume_threads)
    {
        const auto resumed_thread = std::find_if(threads_.begin(), threads_.end(), bind_front(&thread_data::resume));
        if (resumed_thread == threads_.end())
        {
            if (funcs_.size() >= threads_.size()) // spawn new thread
            {
                thread_data data = { worker, this, false };
                if (!data)
                    return false;
                threads_.push_back(std::move(data));
            }

            // todo: (ELSE) check are threads valid
        }
    }
    else if (threads_.empty())
    {
        return false;
    }

    funcs_.emplace_front(std::move(func));
    return true;
}

thread_pool::thread_pool()
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    // FD_ASSERT(info.dwNumberOfProcessors <= std::numeric_limits<uint8_t>::max());
    threads_.reserve(info.dwNumberOfProcessors);
}

thread_pool::~thread_pool()
{
    if (!funcs_.empty())
        this->wait();

#ifdef _DEBUG
    const lock_guard guard = mtx_;
    threads_.clear();
#endif
}

void thread_pool::wait()
{
    //"simple" version of task
    semaphore sem     = { 0, 1 };
    const auto stored = this->store_func([&] {
        sem.release();
    });
    if (stored)
        sem.acquire();
}

bool thread_pool::add_simple(function_type func)
{
    return this->store_func(std::move(func));
}

auto thread_pool::add(function_type func) -> task_type
{
    task_type t       = lockable_task(std::move(func));
    const auto stored = this->store_func([=] {
        t->start();
    });
    if (!stored)
        t = finished_task();
    return t;
}

auto thread_pool::add_lazy(function_type func) -> task_type
{
    auto new_func = [this, fn = std::move(func)](semaphore& sem) mutable {
        const auto stored = this->store_func([&] {
            invoke(fn);
            sem.release();
        });
        if (!stored)
            sem.release();
    };

    return lockable_task(std::move(new_func));
}
