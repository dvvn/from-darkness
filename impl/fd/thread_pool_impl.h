#pragma once

#include <fd/thread_pool.h>

#include <atomic_queue/atomic_queue.h>

#include <Windows.h>

#include <mutex>
#include <semaphore>
#include <vector>

namespace fd
{
    struct finished_task final : task
    {
        void start() override
        {
        }

        void wait() override
        {
        }
    };

    template <typename Fn>
    class lockable_task final : public task
    {
        Fn fn_;
        std::binary_semaphore sem_;

      public:
        template <typename Fn1>
        lockable_task(Fn1&& fn, const bool locked = true)
            : fn_(std::forward<Fn1>(fn))
            , sem_(locked ? 0 : 1)

        {
        }

        void start() override
        {
            if constexpr (invocable<Fn>)
            {
                invoke(fn_);
                // ReSharper disable once CppUnreachableCode
                sem_.release();
            }
            else
            {
                invoke(fn_, sem_);
            }
        }

        void wait() override
        {
            sem_.acquire();
        }
    };

    template <typename Fn>
    lockable_task(Fn&&, bool = true) -> lockable_task<std::decay_t<Fn>>;

    class basic_thread_data
    {
        HANDLE handle_;

      public:
        basic_thread_data(HANDLE handle = INVALID_HANDLE_VALUE);
        explicit operator bool() const;

        bool pause();
        bool resume();
        void terminate();
    };

    class thread_data : public basic_thread_data
    {
        DWORD id_;

      public:
        ~thread_data();
        thread_data(void* fn, void* fnParams, bool suspend);
        thread_data(thread_data&& other) noexcept;

        thread_data(const thread_data& other)            = delete;
        thread_data& operator=(const thread_data& other) = delete;

        thread_data& operator=(thread_data&& other) noexcept;
        bool operator==(DWORD id) const;
    };

    class thread_pool final : public basic_thread_pool
    {
        // using basic_thread_pool::function_type;

        std::vector<thread_data> threads_;
        std::mutex threadsMtx_;

        atomic_queue::AtomicQueue2<function_type, 1024 * 1024 / sizeof(function_type)> funcs_;

        static DWORD WINAPI worker(void* impl) noexcept
        {
            return static_cast<thread_pool*>(impl)->worker_impl();
        }

        bool worker_impl();
        bool store_func(function_type&& func, bool resumeThreads = true) noexcept;

      public:
        thread_pool();
        ~thread_pool() override;

        void wait() override;
        bool add_simple(function_type func) override;
        task_type add(function_type func) override;
        task_type add_lazy(function_type func) override;
    };
} // namespace fd
