module;

#include <Windows.h>

export module fd.mutex;
import fd.memory;

struct mutex_data;

class mutex
{
    fd::unique_ptr<mutex_data> data_;

  public:
    mutex();
    ~mutex();

    void lock() noexcept;
    void unlock() noexcept;
};

template <class M>
class lock_guard
{
    M* mtx_;

  public:
    lock_guard(const lock_guard&) = delete;

    lock_guard(M& mtx)
        : mtx_(&mtx)
    {
        mtx_->lock();
    }

    ~lock_guard()
    {
        if (mtx_)
            mtx_->unlock();
    }

    void release()
    {
        mtx_->unlock();
        mtx_ = nullptr;
    }
};

export namespace fd
{
    using ::lock_guard;
    using ::mutex;
} // namespace fd
