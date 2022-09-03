module;

#include <cstddef>
#include <cstdint>

#include <Windows.h>

export module fd.mutex;

struct basic_mutex
{
    virtual ~basic_mutex() = default;

    basic_mutex() = default;

    basic_mutex(const basic_mutex&)            = delete;
    basic_mutex& operator=(const basic_mutex&) = delete;

    virtual void lock() noexcept   = 0;
    virtual void unlock() noexcept = 0;
};

class mutex final : public basic_mutex
{
    CRITICAL_SECTION sec_;

  public:
    ~mutex() override;
    mutex();

    void lock() noexcept;
    void unlock() noexcept;
};

class recursive_mutex final : public basic_mutex
{
    CRITICAL_SECTION sec_;

  public:
    ~recursive_mutex() override;
    recursive_mutex();

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
    using ::recursive_mutex;
} // namespace fd
