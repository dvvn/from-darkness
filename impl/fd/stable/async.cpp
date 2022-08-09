module;

#include <fd/assert.h>
#include <fd/object.h>

#include <algorithm>
#include <array>
#include <memory>
#include <variant>
#include <vector>

#include <Windows.h>

#include <veque.hpp>

module fd.async;
import fd.lazy_invoke;

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

class custom_thread
{
    HANDLE handle_ = INVALID_HANDLE_VALUE;
    DWORD id_      = 0;

  public:
    ~custom_thread()
    {
        join(); // or force stop
        if (handle_ && handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(handle_);
    }

    constexpr custom_thread() = default;

    custom_thread(void* worker, void* arg)
    {
        handle_ = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(worker), arg, 0, &id_);
    }

    bool joinable() const
    {
        return id_ != 0;
    }

    void join()
    {
        if (!joinable())
            return;

        if (WaitForSingleObjectEx(handle_, INFINITE, FALSE) == WAIT_FAILED)
            FD_ASSERT_UNREACHABLE("Unable to join the thread!");

        id_ = 0;
    }

    DWORD get_id() const
    {
        return id_;
    }

    // must be called at the end of thread
    void _Destroy()
    {
        CloseHandle(handle_);
        handle_ = nullptr;
        id_     = 0;
    }
};

bool operator==(const custom_thread& left, const DWORD right)
{
    return left.get_id() == right;
}

template <class T>
struct threads_storage
{
    using size_type = uint8_t;

  private:
    std::unique_ptr<T[]> threads_;
    size_type count_;

  public:
    threads_storage()
    {
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        FD_ASSERT(info.dwNumberOfProcessors <= std::numeric_limits<size_type>::max())
        count_ = static_cast<size_type>(info.dwNumberOfProcessors);
        std::construct_at(&threads_, new T[count_]);
    }

    T* begin() const
    {
        return threads_.get();
    }

    T* end() const
    {
        return threads_.get() + count_;
    }

    size_type size() const
    {
        return count_;
    }
};

class custom_mutex
{
    CRITICAL_SECTION cs_;

  public:
    custom_mutex()
    {
        InitializeCriticalSection(&cs_);
    }

    ~custom_mutex()
    {
        DeleteCriticalSection(&cs_);
    }

    void lock() noexcept
    {
        EnterCriticalSection(&cs_);
    }

    void unlock() noexcept
    {
        LeaveCriticalSection(&cs_);
    }
};

template <class M>
class mutex_locker
{
    M* mtx_;

  public:
    mutex_locker(const mutex_locker&) = delete;

    mutex_locker(M& mtx)
        : mtx_(&mtx)
    {
        mtx_->lock();
    }

    ~mutex_locker()
    {
        if (mtx_)
            mtx_->unlock();
    }

    void release()
    {
        if (mtx_)
        {
            mtx_->unlock();
            mtx_ = nullptr;
        }
    }
};

class thread_pool_impl final : public basic_thread_pool
{
    using task_type     = std::variant<function_type, function_type_ex>;
    using tasks_storage = veque::veque<task_type>;

    threads_storage<custom_thread> threads_;
    custom_mutex threads_mtx_;
    tasks_storage tasks_;
    custom_mutex tasks_mtx_;
    custom_atomic_bool stop_ = false;

    static DWORD __stdcall worker(void* impl) noexcept
    {
        const auto pool = static_cast<thread_pool_impl*>(impl);

        const fd::lazy_invoke destroyer = [=] {
            const mutex_locker locker(pool->threads_mtx_);
            std::find(pool->threads_.begin(), pool->threads_.end(), GetCurrentThreadId())->_Destroy();
        };

        for (;;)
        {
            if (pool->stop_)
                break;
            mutex_locker locker(pool->tasks_mtx_);
            if (pool->tasks_.empty())
                break;
            auto task = std::move(pool->tasks_.back());
            pool->tasks_.pop_back();
            locker.release();

            std::visit(
                [&]<class Fn>(Fn& fn) {
                    if constexpr (std::same_as<Fn, function_type>)
                        fn();
                    else if constexpr (std::same_as<Fn, function_type_ex>)
                        fn(pool->stop_);
                    else
                        static_assert(std::_Always_false<Fn>);
                },
                task);
        }

        return EXIT_SUCCESS;
    }

    void store_task_impl(task_type&& t)
    {
        const mutex_locker locker(tasks_mtx_);
        tasks_.emplace_back(std::move(t));
    }

    void spawn_thread_impl()
    {
        const mutex_locker locker(threads_mtx_);
        const auto end      = threads_.end();
        const auto finished = std::find_if_not(threads_.begin(), end, fd::bind_front(&custom_thread::joinable));
        if (finished == end)
            return;
        // std::destroy_at(finished);
        std::construct_at(finished, worker, this);
    }

    template <class Fn>
    void store_task(Fn& func) noexcept
    {
        store_task_impl(std::move(func));
        spawn_thread_impl();
    }

  public:
    thread_pool_impl() = default;

    ~thread_pool_impl()
    {
        stop_ = true;
    }

    bool contains_thread(const DWORD id) const
    {
        const auto end = threads_.end();
        return std::find(threads_.begin(), end, id) != end;
    }

    void wait() override
    {
        for (auto& t : threads_)
        {
            if (t.joinable())
                t.join();
        }
    }

    task operator()(function_type func) override
    {
        store_task(func);
        return {};
    }

    task operator()(function_type_ex func) override
    {
        store_task(func);
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
