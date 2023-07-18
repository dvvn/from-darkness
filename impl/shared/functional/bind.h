#pragma once

#include "noncopyable.h"

#include <functional>

namespace fd
{
using std::bind;
using std::bind_back;
using std::bind_front;

//todo: add flags to invoke on copy/move/destruct
template <
    typename Fn,
    bool Trivial = std::equality_comparable_with<Fn, nullptr_t> && /**/ std::assignable_from<Fn &, nullptr_t>>
class invoke_on_destruct;

template <typename Fn>
class invoke_on_destruct<Fn, true> final : public noncopyable
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

    void reset()
    {
        fn_ = nullptr;
    }
};

template <typename Fn>
class invoke_on_destruct<Fn, false> final : public noncopyable
{
    bool has_value_;

    union
    {
        Fn fn_;
    };

  public:
    ~invoke_on_destruct()
    {
        if (!has_value_)
            return;
        static_cast<Fn &&>(fn_)();
        if constexpr (!std::is_trivially_destructible_v<Fn>)
            std::destroy_at(&fn_);
    }

    invoke_on_destruct(Fn fn)
        : has_value_(true)
        , fn_(std::move(fn))
    {
    }

    invoke_on_destruct(invoke_on_destruct &&other) noexcept
        : has_value_(std::exchange(other.has_value_, false))
        , fn_(std::move(other.fn_))
    {
    }

    invoke_on_destruct &operator=(invoke_on_destruct &&other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        swap(has_value_, other.has_value_);
        return *this;
    }

    void reset()
    {
        fn_.reset();
    }
};

template <typename Fn>
invoke_on_destruct(Fn) -> invoke_on_destruct<std::decay_t<Fn>>;
}