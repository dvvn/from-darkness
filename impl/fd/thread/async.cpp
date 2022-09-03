module;

#include <fd/assert.h>
#include <fd/object.h>

#include <veque.hpp>

#include <Windows.h>

#include <algorithm>
#include <array>
#include <list>
#include <variant>
#include <vector>

module fd.async;
import fd.functional.bind;
import fd.mutex;
import fd.semaphore;

using namespace fd;

struct finished_task_data : basic_task_data
{
    finished_task_data() = default;

    void on_start() override
    {
    }

    void on_wait() override
    {
    }
};

class task_data : public basic_task_data
{
    function_type fn;
    semaphore sm;

  public:
    task_data(function_type&& fn)
        : fn(std::move(fn))
        , sm(0, 1)
    {
    }

    void on_start() override
    {
        invoke(fn);
        sm.release();
    }

    void on_wait() override
    {
        sm.acquire();
    }
};

struct manual_task_data : basic_task_data
{
    function_type fn;
    semaphore sm;

    manual_task_data()
        : sm(1, 1)
    {
    }

    void on_start() override
    {
        sm.acquire();
        invoke(fn);
        // call release manually from fn
    }

    void on_wait() override
    {
        sm.acquire();
    }
};

basic_task_data* task::_Data() const
{
    return data_.get();
}

void task::start()
{
    data_->on_start();
}

void task::wait()
{
    data_->on_wait();
}

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

class thread_pool_impl final : public basic_thread_pool
{
    struct thread_data
    {
        HANDLE h;
        DWORD id;
        bool paused;

        bool operator==(const DWORD id) const
        {
            return this->id == id;
        }
    };

    std::vector<thread_data> threads_;

    using task_type = function_type /* std::variant<function_type, function_type_ex> */;

    /*std::list*/ veque::veque<task_type> tasks_;
    recursive_mutex mtx_;

    static DWORD __stdcall worker(void* impl) noexcept
    {
        const auto pool        = static_cast<thread_pool_impl*>(impl);
        const auto this_thread = std::find(pool->threads_.begin(), pool->threads_.end(), GetCurrentThreadId());

        for (;;)
        {
            pool->mtx_.lock();
            if (pool->tasks_.empty())
            {
                pool->mtx_.unlock();
                FD_ASSERT(!this_thread->paused);
                this_thread->paused = true;
                SuspendThread(this_thread->h);
            }
            else
            {
                auto task = std::move(pool->tasks_.back());
                pool->tasks_.pop_back();
                pool->mtx_.unlock();

                try
                {
                    invoke(task);
                    /* std::visit(
                    [=]<class Fn>(Fn& fn) {
                        if constexpr (std::same_as<Fn, function_type>)
                            fn();
                        else if constexpr (std::same_as<Fn, function_type_ex>)
                            fn(pool->stop_);
                        else
                            static_assert(std::_Always_false<Fn>);
                    },
                    task); */
                }
                catch (...)
                {
                    return TRUE;
                }
            }
        }
    }

    bool store_func(function_type&& func, const bool resume_threads = true) noexcept
    {
        const lock_guard guard(mtx_);

#ifdef _DEBUG
        if (threads_.empty())
            return false;
#endif

        tasks_.emplace_front(std::move(func));

        const auto threads_begin = threads_.begin();
        const auto threads_end   = threads_.end();
        auto paused_thread       = std::find_if(threads_begin, threads_end, bind_front(&thread_data::paused));
        if (paused_thread == threads_end)
            return true;

        const auto tasks_in_queue   = tasks_.size() - 1;
        const size_t active_threads = std::distance(threads_begin, paused_thread);
        if (tasks_in_queue < active_threads)
            return true;

        if (resume_threads)
        {
            for (; paused_thread != threads_end; ++paused_thread)
            {
                if (ResumeThread(paused_thread->h))
                {
                    paused_thread->paused = false;
                    return true;
                }
            }
        }
        if (active_threads == 0)
        {
            tasks_.pop_front();
            return false;
        }

        return true;
    }

  public:
    thread_pool_impl()
    {
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        // FD_ASSERT(info.dwNumberOfProcessors <= std::numeric_limits<uint8_t>::max());
        threads_.resize(info.dwNumberOfProcessors);
        for (auto& t : threads_)
        {
            t.h      = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(worker), this, CREATE_SUSPENDED, &t.id);
            t.paused = true;
        }
    }

    ~thread_pool_impl()
    {
        if (!tasks_.empty())
            this->wait();

#ifdef _DEBIG
        const lock_guard guard(mtx_);
#endif

        for (auto& t : threads_)
        {
            TerminateThread(t.h, EXIT_SUCCESS);
            CloseHandle(t.h);
        }

#ifdef _DEBUG
        threads_.clear();
#endif
    }

    bool contains_thread(const DWORD id) const
    {
        const auto end = threads_.end();
        return std::find(threads_.begin(), end, id) != end;
    }

    void wait() override
    {
        //"simple" version of task
        semaphore sem(0, 1);
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
        task t(task_data(std::move(func)));

        const auto stored = this->store_func([=]() mutable {
            t.start();
        });
        if (stored)
            return t;
        return finished_task_data();
    }

    task operator()(function_type func, const lazy_tag_t) override
    {
        auto t = task(manual_task_data());

        auto data = static_cast<manual_task_data*>(t._Data());
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

FD_OBJECT_BIND_TYPE(async, thread_pool_impl);
