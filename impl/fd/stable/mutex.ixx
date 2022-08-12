module;

#include <Windows.h>

export module fd.mutex;
import fd.memory;

struct mutex_data;

class basic_mutex
{
  private:
    fd::unique_ptr<mutex_data> data_;

  protected:
    mutex_data* data() const;

  public:
    basic_mutex();
    virtual ~basic_mutex();

    virtual void lock() noexcept;
    virtual void unlock() noexcept;
};

struct recursive_mutex final : basic_mutex
{
};

struct mutex final : basic_mutex
{
    void lock() noexcept override;
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
