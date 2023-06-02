#pragma once

#include <boost/core/noncopyable.hpp>

#include <optional>

namespace fd
{
template <
    typename Fn,
    bool Trivial = std::equality_comparable_with<Fn, nullptr_t> && /**/ std::assignable_from<Fn &, nullptr_t>>
class invoke_on_destruct;

template <typename Fn>
class invoke_on_destruct<Fn, true> final : public boost::noncopyable
{
    Fn fn_;

  public:
    ~invoke_on_destruct()
    {
        if (fn_ != nullptr)
            fn_();
    }

    invoke_on_destruct(Fn fn)
        : fn_(std::move(fn))
    {
    }

    invoke_on_destruct(invoke_on_destruct &&other) noexcept
        : fn_(std::exchange(other.fn_, nullptr))
    {
    }

    invoke_on_destruct &operator=(invoke_on_destruct &&other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }

    invoke_on_destruct &operator=(nullptr_t) noexcept
    {
        fn_ = nullptr;
        return *this;
    }
};

template <typename Fn>
class invoke_on_destruct<Fn, false> final : public boost::noncopyable
{
    std::optional<Fn> fn_;

  public:
    ~invoke_on_destruct()
    {
        if (fn_.has_value())
            (*fn_)();
    }

    invoke_on_destruct(Fn fn)
        : fn_(std::move(fn))
    {
    }

    invoke_on_destruct(invoke_on_destruct &&other) noexcept
        : fn_(std::exchange(other.fn_, std::nullopt))
    {
    }

    invoke_on_destruct &operator=(invoke_on_destruct &&other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }

    invoke_on_destruct &operator=(nullptr_t)
    {
        fn_.reset();
        return *this;
    }
};

template <typename Fn>
invoke_on_destruct(Fn) -> invoke_on_destruct<std::decay_t<Fn>>;
}