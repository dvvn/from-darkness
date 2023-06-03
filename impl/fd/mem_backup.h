#pragma once

#include "core.h"

#include <boost/core/noncopyable.h>

#include <cassert>

namespace fd
{
template <typename T>
struct mem_backup : boost::noncopyable
{
    using value_type = T;
    using pointer    = T *;
    using reference  = _const<T> &;

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

    mem_backup(mem_backup &&other) noexcept
        : value_(std::move(other.value_))
        , owner_(std::exchange(other_.owner_, nullptr))
    {
    }

    mem_backup &operator=(mem_backup &&other) noexcept
    {
        using std::swap;
        swap(value_, other.value_);
        swap(owner_, other.owner_);
        return *this;
    }

    mem_backup() = default;

    mem_backup(T &from)
    {
        backup_.emplace(from, &from);
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
} // namespace fd
