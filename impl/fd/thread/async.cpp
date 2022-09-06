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

class thread_data
{
    HANDLE handle_;
    DWORD id_;
    bool paused_;

    thread_data(const thread_data&)            = default;
    thread_data& operator=(const thread_data&) = default;

  public:
    thread_data(void* fn, void* fn_owner)
    {
        handle_ = CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(fn), fn_owner, CREATE_SUSPENDED, &id_);
        paused_ = true;
    }

    ~thread_data()
    {
        if (handle_)
        {
            TerminateThread(handle_, EXIT_SUCCESS);
            CloseHandle(handle_);
        }
    }

    thread_data(thread_data&& other)
    {
        *this         = other;
        other.handle_ = nullptr;
    }

    thread_data& operator=(thread_data&& other)
    {
        auto tmp = other;
        other    = *this;
        *this    = tmp;
        return *this;
    }

    bool operator==(const DWORD id) const
    {
        return this->id_ == id;
    }

    bool operator==(const thread_data& other) const
    {
        return this->id_ == other.id_;
    }

    bool pause()
    {
        FD_ASSERT(!paused_);
        paused_ = true;
        return SuspendThread(handle_);
    }

    bool paused() const
    {
        return paused_;
    }

    bool resume()
    {
        FD_ASSERT(paused_);
        const auto ok = ResumeThread(handle_);
        paused_       = false;
        return ok;
    }
};

class thread_pool_impl final : public async
{
    std::vector<thread_data> threads_;

    using task_type = function_type /* std::variant<function_type, function_type_ex> */;

#ifdef VEQUE_HEADER_GUARD
    veque::veque
#else
    std::deque
#endif
        <task_type>
            tasks_;
    mutable recursive_mutex mtx_;

    static DWORD WINAPI worker(void* impl) noexcept
    {
        const auto pool        = static_cast<thread_pool_impl*>(impl);
        const auto this_thread = std::find(pool->threads_.begin(), pool->threads_.end(), GetCurrentThreadId());

        for (;;)
        {
            pool->mtx_.lock();
            if (pool->tasks_.empty())
            {
                pool->mtx_.unlock();

                if (!this_thread->pause())
                    return FALSE;
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
                    // todo: any external exception -> false
                    // sfecial 'interrupt' exception -> true
                    return TRUE;
                }
            }
        }
    }

    bool store_func(function_type&& func, const bool resume_threads = true) noexcept
    {
        const lock_guard guard(mtx_);

        if (threads_.empty())
            return false;

        const size_t active_threads = std::count_if(threads_.begin(), threads_.end(), bind_front(inverse_result(&thread_data::paused)));

        lazy_invoke store_func = [&] {
            tasks_.emplace_front(std::move(func));
        };

        if (resume_threads)
        {
            const auto tasks_in_queue = tasks_.size();
            if (tasks_in_queue < active_threads)
                return true;
            if (active_threads == threads_.size())
                return true;
            for (auto& t : threads_)
            {
                if (t.paused() && t.resume())
                    return true;
            }
        }
        if (active_threads == 0)
        {
            store_func.reset();
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
        threads_.reserve(info.dwNumberOfProcessors);
        while (threads_.size() != info.dwNumberOfProcessors)
            threads_.emplace_back(worker, this);
    }

    ~thread_pool_impl()
    {
        if (!tasks_.empty())
            this->wait();

#ifdef _DEBUG
        const lock_guard guard(mtx_);
        threads_.clear();
#endif
    }

    bool contains_thread(const DWORD id) const
    {
        const lock_guard guard(mtx_);
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
