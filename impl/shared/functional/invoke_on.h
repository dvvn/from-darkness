#pragma once

#include "optional.h"
#include "overload.h"

#include <functional>

namespace fd
{
enum class class_state : uint8_t
{
    construct,
    destruct,
    copy,
    move
};

template <class_state State>
using class_state_constant = std::integral_constant<class_state, State>;

template <class_state State, typename Callback>
class invoke_on;

template <class_state State, typename Callback>
constexpr auto make_invoke_on(class_state_constant<State>, Callback&& callback) -> invoke_on<State, std::decay_t<Callback>>
{
    return {std::forward<Callback>(callback)};
}

namespace detail
{
template <typename Callback>
concept nullable_callback_check = //
    !std::is_class_v<Callback> || //
    requires(Callback c) {
        {
            c.operator!()
        };
    } || //
    requires(Callback c) {
        {
            operator!(c)
        };
    };

template <typename Callback, bool = nullable_callback_check<Callback>>
struct nullable_callback;

template <typename Callback>
struct nullable_callback<Callback, true> : overload_t<Callback>
{
    overload_t<Callback>& operator*()
    {
        return *this;
    }

    overload_t<Callback> const& operator*() const
    {
        return *this;
    }

    bool has_value() const
    {
        return !!(operator*());
    }

    template <typename... Args>
    void operator()(Args&&... args) const
    {
        if (has_value())
            std::invoke(operator*(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args)
    {
        if (has_value())
            std::invoke(operator*(), std::forward<Args>(args)...);
    }
};

template <typename Callback>
struct nullable_callback<Callback, false> : optional<Callback>
{
    using optional<Callback>::operator*;
    using optional<Callback>::has_value;

    template <typename... Args>
    void operator()(Args&&... args) const
    {
        if (has_value())
            std::invoke(operator*(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args)
    {
        if (has_value())
            std::invoke(operator*(), std::forward<Args>(args)...);
    }
};
} // namespace detail

template <typename Callback>
class invoke_on<class_state::construct, Callback>
{
  public:
    template <std::constructible_from<Callback> Fn>
    constexpr invoke_on(Fn&& callback)
    {
        std::invoke(std::forward<Fn>(callback));
    }
};

template <typename Callback>
class invoke_on<class_state::destruct, Callback>
{
    detail::nullable_callback<Callback> callback_;

  public:
    ~invoke_on()
    {
        if (callback_)
            std::invoke(callback_);
    }

    template <std::constructible_from<Callback> Fn>
    constexpr invoke_on(Fn&& callback)
        : callback_(std::forward<Fn>(callback))
    {
    }
};

template <typename Callback>
class invoke_on<class_state::copy, Callback>
{
    Callback callback_;

  public:
    template <std::constructible_from<Callback> Fn>
    constexpr invoke_on(Fn&& callback)
        : callback_(std::forward<Fn>(callback))
    {
    }

    invoke_on(invoke_on const& other)
        : callback_(other.callback_)
    {
        std::invoke(callback_);
    }

    invoke_on& operator=(invoke_on const& other)
    {
        callback_ = other.callback_;
        std::invoke(callback_);
        return *this;
    }

    invoke_on(invoke_on&& other) noexcept
        : callback_(std::move(other.callback_))
    {
    }

    invoke_on& operator=(invoke_on&& other) noexcept
    {
        using std::swap;
        swap(callback_, other.callback_);
        return *this;
    }
};

template <typename Callback>
class invoke_on<class_state::move, Callback>
{
    detail::nullable_callback<Callback> callback_;

  public:
    template <std::constructible_from<Callback> Fn>
    constexpr invoke_on(Fn&& callback)
        : callback_(std::forward<Fn>(callback))
    {
    }

    invoke_on(invoke_on const& other)            = default;
    invoke_on& operator=(invoke_on const& other) = default;

    invoke_on(invoke_on&& other) noexcept
        : callback_(std::move(other.callback_))
    {
        std::invoke(callback_);
    }

    invoke_on& operator=(invoke_on&& other) noexcept
    {
        using std::swap;
        swap(callback_, other.callback_);
        std::invoke(callback_);
        return *this;
    }
};
}