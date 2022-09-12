module;

#include <fd/assert.h>
#include <fd/object.h>

#include <veque.hpp>

#include <Windows.h>

#include <algorithm>
#include <array>
#include <deque>
#include <variant>
#include <vector>

module fd.async;
import fd.functional.bind;
import fd.functional.lazy_invoke;
import fd.mutex;
import fd.semaphore;

using namespace fd;

class task_data : public basic_task
{
    function_type fn;
    semaphore sm;

  public:
    task_data(function_type&& fn)
        : fn(std::move(fn))
        , sm(0, 1)
    {
    }

    void start() override
    {
        invoke(fn);
        sm.release();
    }

    void wait() override
    {
        sm.acquire();
    }
};

struct manual_task_data : basic_task
{
    function_type fn;
    semaphore sm;

    manual_task_data()
        : sm(0, 1)
    {
    }

    void start() override
    {
        invoke(fn); // call release manually from fn
    }

    void wait() override
    {
        sm.acquire();
    }
};

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

class basic_thread_data
{
    HANDLE handle_;

    friend class thread_data;

  public:
    basic_thread_data(HANDLE handle = INVALID_HANDLE_VALUE)
        : handle_(handle)
    {
    }

    explicit operator bool() const
    {
        return handle_ && handle_ != INVALID_HANDLE_VALUE;
    }

    bool pause()
    {
        return SuspendThread(handle_);
    }

    bool resume()
    {
        return ResumeThread(handle_);
    }
};

class thread_data : public basic_thread_data
{
    DWORD id_;

    basic_thread_data* _Base()
    {
        return this;
    }

  public:
    ~thread_data()
    {
        auto& base = *_Base();
        if (base)
        {
            TerminateThread(base.handle_, EXIT_SUCCESS);
            CloseHandle(base.handle_);
        }
    }

    thread_data(void* fn, void* fn_params, const bool suspend)
        : basic_thread_data(CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(fn), fn_params, suspend ? CREATE_SUSPENDED : 0, &id_))
    {
    }

    thread_data(thread_data&& other)
        : basic_thread_data(other)
    {
        *_Base() = INVALID_HANDLE_VALUE;
        id_      = other.id_;
    }

    thread_data& operator=(thread_data&& other)
    {
        using std::swap;
        swap(*_Base(), *other._Base());
        swap(id_, other.id_);
        return *this;
    }

    bool operator==(const DWORD id) const
    {
        return this->id_ == id;
    }

    /* bool operator==(const thread_data& other) const
    {
        return this->id_ == other.id_;
    } */
};

class thread_pool_impl final : public async
{
    using threads_storage = std::vector<thread_data>;

    using task_type = function_type /* std::variant<function_type, function_type_ex> */;
    using tasks_storage =
#ifdef VEQUE_HEADER_GUARD
        veque::veque
#else
        std::deque
#endif
        <task_type>;
    using mutex_type = recursive_mutex;

    //---

    threads_storage threads_;
    tasks_storage tasks_;
    mutable mutex_type mtx_;

    bool _Worker() noexcept
    {
        basic_thread_data this_thread;

        const auto callback = [&]() -> uint8_t {
            if (tasks_.empty())
            {
                mtx_.unlock();
                if (!this_thread.pause())
                    return 0;
            }
            else
            {
                auto task = std::move(tasks_.back());
                tasks_.pop_back();
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

    static DWORD WINAPI worker(void* impl)
    {
        const auto pool = static_cast<thread_pool_impl*>(impl);
        return pool->_Worker();
    }

    bool store_func(function_type&& func, const bool resume_threads = true) noexcept
    {
        const lock_guard guard = mtx_;

        if (resume_threads)
        {
            const auto resumed_thread = std::find_if(threads_.begin(), threads_.end(), bind_front(&thread_data::resume));
            if (resumed_thread == threads_.end())
            {
                if (tasks_.size() >= threads_.size())
                {
                    thread_data data = { worker, this, false };
                    if (!data)
                        return false;
                    threads_.push_back(std::move(data));
                }

                // todo:  check are threads valid
            }
        }
        else if (threads_.empty())
        {
            return false;
        }

        tasks_.emplace_front(std::move(func));

        return true;
    }

  public:
    thread_pool_impl()
    {
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        // FD_ASSERT(info.dwNumberOfProcessors <= std::numeric_limits<uint8_t>::max());
        threads_.reserve(info.dwNumberOfProcessors);
    }

    ~thread_pool_impl()
    {
        if (!tasks_.empty())
            this->wait();

#ifdef _DEBUG
        const lock_guard guard = mtx_;
        threads_.clear();
#endif
    }

    void wait() override
    {
        //"simple" version of task
        semaphore sem     = { 0, 1 };
        const auto stored = this->store_func([&] {
            sem.release();
        });
        if (stored)
            sem.acquire();
    }

    bool operator()(function_type func, const simple_tag_t) override
    {
        return this->store_func(std::move(func));
    }

    task operator()(function_type func) override
    {
        task t = task_data(std::move(func));

        const auto stored = this->store_func([=] {
            t->start();
        });
        if (stored)
            return t;
        return finished_task();
    }

    task operator()(function_type func, const lazy_tag_t) override
    {
        task t = manual_task_data();

        auto data = static_cast<manual_task_data*>(t.get());
        data->fn  = [=, fn = std::move(func)]() mutable {
            const auto stored = this->store_func([&] {
                invoke(fn);
                data->sm.release();
            });
            if (!stored)
                data->sm.release();
        };
        return t;
    }
};

/* void thread_sleep(const size_t ms)
{
#ifdef _DEBUG
    const auto tid = GetCurrentThreadId();
    if (!FD_OBJECT_GET(thread_pool_impl)->contains_thread(tid))
        FD_ASSERT("Unable to pause non-owning thread");
#endif

    Sleep(ms);
} */

FD_OBJECT_ATTACH(async, thread_pool_impl);
