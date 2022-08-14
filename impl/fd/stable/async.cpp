module;

#include <fd/assert.h>
#include <fd/object.h>

#include <algorithm>
#include <array>
#include <variant>
#include <vector>

#include <Windows.h>

#include <veque.hpp>

module fd.async;
import fd.functional.lazy_invoke;
import fd.functional.bind;
import fd.mutex;

custom_atomic_bool::custom_atomic_bool(const bool value)
    : value_(value)
{
}

custom_atomic_bool::operator bool() const
{
    return static_cast<bool>(InterlockedOr8((volatile char*)(&value_), 0));
}

custom_atomic_bool& custom_atomic_bool::operator=(const bool value)
{
    InterlockedExchange8((volatile char*)(&value_), static_cast<char>(value));
    return *this;
}

class thread_pool_impl final : public basic_thread_pool
{
    struct thread_data
    {
        HANDLE h;
        DWORD id;
        bool paused;
    };

    std::vector<thread_data> threads_;

    using task_type = std::variant<function_type, function_type_ex>;

    veque::veque<task_type> tasks_;
    fd::mutex tasks_mtx_;

    custom_atomic_bool stop_ = false;

    static DWORD __stdcall worker(void* impl) noexcept
    {
        const auto pool        = static_cast<thread_pool_impl*>(impl);
        const auto this_thread = std::find_if(pool->threads_.begin(), pool->threads_.end(), [this_id = GetCurrentThreadId()](const auto& d) {
            return d.id == this_id;
        });

        while (!pool->stop_)
        {
            pool->tasks_mtx_.lock();
            if (pool->tasks_.empty())
            {
                pool->tasks_mtx_.unlock();
                if (!this_thread->paused)
                    SuspendThread(this_thread->h);
            }
            else
            {
                auto task = std::move(pool->tasks_.back());
                pool->tasks_.pop_back();
                pool->tasks_mtx_.unlock();

                std::visit(
                    [=]<class Fn>(Fn& fn) {
                        if constexpr (std::same_as<Fn, function_type>)
                            fn();
                        else if constexpr (std::same_as<Fn, function_type_ex>)
                            fn(pool->stop_);
                        else
                            static_assert(std::_Always_false<Fn>);
                    },
                    task);
            }
        }

        return EXIT_SUCCESS;
    }

    template <class Fn>
    void store_func(Fn& func) noexcept
    {
        const fd::lock_guard locker(tasks_mtx_);
        tasks_.emplace_front(std::move(func));

        const auto threads_begin = threads_.begin();
        const auto threads_end   = threads_.end();
        const auto paused_thread = std::find_if(threads_begin, threads_end, fd::bind_front(&thread_data::paused));
        if (paused_thread == threads_end)
            return;

        const auto tasks_in_queue   = tasks_.size() - 1;
        const size_t active_threads = std::distance(threads_begin, paused_thread);
        if (tasks_in_queue < active_threads)
            return;

        ResumeThread(paused_thread->h);
        paused_thread->paused = false;
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
        stop_ = true;
        for (auto& t : threads_)
        {
            if (t.paused)
                TerminateThread(t.h, EXIT_SUCCESS);
            else if (WaitForSingleObjectEx(t.h, INFINITE, FALSE) == WAIT_FAILED)
                FD_ASSERT_UNREACHABLE("Unable to join the thread!");
            CloseHandle(t.h);
        }
    }

    bool contains_thread(const DWORD id) const
    {
        for (auto& t : threads_)
        {
            if (t.id == id)
                return true;
        }
        return false;
    }

    void wait() override
    {
        /* for (auto& t : threads_)
        {
            if (t.joinable())
                t.join();
        } */
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    task operator()(function_type func) override
    {
        store_func(func);
        return {};
    }

    task operator()(function_type_ex func) override
    {
        store_func(func);
        return {};
    }

    task operator()(function_type func, const lazy_tag_t) override
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }

    task operator()(function_type_ex func, const lazy_tag_t) override
    {
        FD_ASSERT_UNREACHABLE("Not implemented");
    }
};

void thread_sleep(const size_t ms)
{
#ifdef _DEBUG
    const auto tid = GetCurrentThreadId();
    if (!FD_OBJECT_GET(thread_pool_impl)->contains_thread(tid))
        FD_ASSERT("Unable to pause non-owning thread");
#endif

    Sleep(ms);
}

FD_OBJECT_BIND_TYPE(async, thread_pool_impl);
