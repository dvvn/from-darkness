module;

#include <atomic_queue/atomic_queue.h>

#include <Windows.h>

#include <mutex>
#include <vector>

export module fd.thread_pool.impl;
export import fd.thread_pool;

namespace fd
{
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
        thread_data(void* fn, void* fn_params, const bool suspend);
        thread_data(thread_data&& other);

        thread_data& operator=(thread_data&& other);
        bool operator==(const DWORD id) const;
    };

    export class thread_pool final : public basic_thread_pool
    {
        // using basic_thread_pool::function_type;

        std::vector<thread_data> threads_;
        std::mutex threads_mtx_;

        atomic_queue::AtomicQueue2<function_type, 128> funcs_;

        static DWORD WINAPI worker(void* impl) noexcept
        {
            return static_cast<thread_pool*>(impl)->worker_impl();
        }

        bool worker_impl();
        bool store_func(function_type&& func, const bool resume_threads = true) noexcept;

      public:
        thread_pool();
        ~thread_pool();

        void wait() override;
        bool add_simple(function_type func) override;
        task_type add(function_type func) override;
        task_type add_lazy(function_type func) override;
    };
} // namespace fd
