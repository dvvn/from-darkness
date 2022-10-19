module;

#include <cstddef>
#include <cstdint>

#include <Windows.h>

export module fd.mutex;
export import fd.lock_guard;

export struct basic_mutex
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

export namespace fd
{
    using ::mutex;
    using ::recursive_mutex;
} // namespace fd
