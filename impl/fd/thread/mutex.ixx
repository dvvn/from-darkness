module;

#include <cstddef>
#include <cstdint>

export module fd.mutex;

#ifdef _WIN32
constexpr size_t mutex_size           = 24;
constexpr size_t recursive_mutex_size = 24;
#else
#endif

struct basic_mutex
{
    virtual ~basic_mutex() = default;

    basic_mutex()                   = default;
    basic_mutex(const basic_mutex&) = delete;

    virtual void lock() noexcept   = 0;
    virtual void unlock() noexcept = 0;
};

class mutex final : public basic_mutex
{
    uint8_t buff_[mutex_size];

  public:
    ~mutex() override;
    mutex();

    void lock() noexcept;
    void unlock() noexcept;
};

class recursive_mutex final : public basic_mutex
{
    uint8_t buff_[recursive_mutex_size];

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
