#pragma once

#include "optional.h"
#include "functional/overload.h"

#include <functional>

namespace fd
{
namespace detail
{
template <typename Callback, bool = /*!std::is_class_v<Callback> ||*/ std::convertible_to<Callback, bool>>
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
        return static_cast<bool>(static_cast<overload_t<Callback> const*>(this));
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

enum object_state : uint8_t
{
    construct = 1 << 0,
    destruct  = 1 << 1,
    copy      = 1 << 2,
    move      = 1 << 3,
};

template <auto State>
using object_state_constant = std::integral_constant<object_state, static_cast<object_state>(State)>;

template <object_state State, typename Callback>
class invoke_on
{
    using nullable_callback = detail::nullable_callback<Callback>;
    using callback          = Callback;

    static constexpr bool store_nullable = State & move || (State & destruct && std::movable<Callback>);
    static constexpr bool store          = State & (copy | destruct);

    using callback_stored = std::conditional_t<store_nullable, nullable_callback, std::conditional_t<store, callback, std::false_type>>;

    [[no_unique_address]] //
    callback_stored callback_;

    template <object_state CurrentState>
    void do_call()
    {
        if constexpr (State & CurrentState)
        {
            if constexpr (!(CurrentState & construct) && std::convertible_to<callback_stored, bool>)
                if (!callback_)
                    return;
            callback_();
        }
    }

  public:
    ~invoke_on()
    {
        do_call<destruct>();
    }

    template <typename Fn>
    constexpr invoke_on(Fn&& callback) requires(State == construct && std::constructible_from<Callback, Fn>)
    {
        std::invoke(std::forward<Fn>(callback));
    }

    template <typename Fn>
    constexpr invoke_on(Fn&& callback) requires(std::constructible_from<callback_stored, Fn>)
        : callback_(std::forward<Fn>(callback))
    {
        do_call<construct>();
    }

    template <class Fn>
    constexpr invoke_on(object_state_constant<State>, Fn&& callback)
        : invoke_on(std::forward<Fn>(callback))
    {
    }

    invoke_on(invoke_on const& other)
        : callback_(other.callback_)
    {
        do_call<copy>();
    }

    invoke_on& operator=(invoke_on const& other)
    {
        callback_ = other.callback_;
        do_call<copy>();
        return *this;
    }

    invoke_on(invoke_on&& other) noexcept
        : callback_(std::move(other.callback_))
    {
        do_call<move>();
    }

    invoke_on& operator=(invoke_on&& other) noexcept
    {
        using std::swap;
        swap(callback_, other.callback_);
        other.do_call<move>();
        do_call<move>();
        return *this;
    }
};

template <object_state State, typename Callback>
invoke_on(object_state_constant<State>, Callback&& callback) -> invoke_on<State, std::decay_t<Callback>>;
}