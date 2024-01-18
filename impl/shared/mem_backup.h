#pragma once

#include <cassert>
#include <utility>

namespace fd
{
template <typename T>
struct mem_backup : noncopyable
{
    using value_type = T;
    using pointer    = T*;
    using reference  = T const&;

  private:
    value_type value_;
    pointer owner_;

    void restore_impl()
    {
        *owner_ = std::move(value_);
    }

  public:
    ~mem_backup()
    {
        if (has_value())
            restore_impl();
    }

    mem_backup(mem_backup&& other) noexcept
        : value_(std::move(other.value_))
        , owner_(std::exchange(other.owner_, nullptr))
    {
    }

    mem_backup& operator=(mem_backup&& other) noexcept
    {
        using std::swap;
        swap(value_, other.value_);
        swap(owner_, other.owner_);
        return *this;
    }

    mem_backup()
        : owner_(nullptr)
    {
    }

    mem_backup(T& from)
        : value_(from)
        , owner_(&from)
    {
    }

    /* template <typename T1>
    mem_backup(T& from, T1&& owerride) //use std::exchange
        : mem_backup(from)
    {
        from = T(std::forward<T1>(owerride));
    } */

    reference get() const
    {
        assert(has_value());
        return value_;
    }

    void restore()
    {
        if (has_value())
        {
            restore_impl();
            reset();
        }
    }

    void reset()
    {
        owner_ = nullptr;
    }

    bool has_value() const
    {
        return owner_ != nullptr;
    }
};

template <typename T>
struct mem_backup<T const>
{
    mem_backup(auto&&...) = delete;
};

template <typename T>
mem_backup(T&) -> mem_backup<T>;

template <typename T>
auto make_mem_backup(T& value) -> mem_backup<T>
{
    return value;
}

template <typename T, typename Q>
auto make_mem_backup(T& value, Q&& replace) -> mem_backup<T>
{
    mem_backup<T> backup(value);

    value = std::forward<Q>(replace);
    return std::move(backup);
}
} // namespace fd