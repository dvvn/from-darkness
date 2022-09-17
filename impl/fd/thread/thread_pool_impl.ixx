module;

#if __has_include(<veque.hpp>)
#include <veque.hpp>
#endif

#include <Windows.h>

#ifndef VEQUE_HEADER_GUARD
#include <deque>
#endif
#include <vector>

export module fd.thread_pool.impl;
export import fd.thread_pool;
import fd.mutex;

using namespace fd;

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

class thread_pool final : public basic_thread_pool
{
    // using basic_thread_pool::function_type;

    using threads_storage = std::vector<thread_data>;

    using funcs_storage =
#ifdef VEQUE_HEADER_GUARD
        veque::veque
#else
        std::deque
#endif
        <function_type>;
    using mutex_type = recursive_mutex;

    //---

    threads_storage threads_;
    funcs_storage funcs_;
    mutable mutex_type mtx_;

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
    bool operator()(function_type func, const simple_tag_t) override;
    task_type operator()(function_type func) override;
    task_type operator()(function_type func, const lazy_tag_t) override;
};

export namespace fd
{
    using ::thread_pool;
} // namespace fd
