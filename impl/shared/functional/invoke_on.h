#pragma once
#include "type_traits/conditional.h"
#include "type_traits/integral_constant.h"
#include "optional.h"

#include <cassert>
#include <functional>

namespace fd
{
namespace detail
{
template <typename Callback, bool = /*!std::is_class_v<Callback> ||*/ std::convertible_to<Callback, bool>>
class basic_invoke_on_wrapped_callback;

template <typename Callback>
class basic_invoke_on_wrapped_callback<Callback, true>
{
    Callback callback_;

  public:
    template <typename Fn>
    basic_invoke_on_wrapped_callback(Fn&& callback) requires(std::constructible_from<Callback, Fn &&>)
        : callback_{std::forward<Fn>(callback)}
    {
    }

    bool has_value() const
    {
        return static_cast<bool>(callback_);
    }

    explicit operator bool() const
    {
        return static_cast<bool>(callback_);
    }

    Callback& operator*()
    {
        assert(has_value());
        return callback_;
    }

    Callback const& operator*() const
    {
        assert(has_value());
        return callback_;
    }
};

template <typename Callback>
class basic_invoke_on_wrapped_callback<Callback, false> : optional<Callback>
{
  public:
    template <typename Fn>
    basic_invoke_on_wrapped_callback(Fn&& callback) requires(std::constructible_from<Callback, Fn &&>)
        : optional<Callback>{std::forward<Fn>(callback)}
    {
    }

    using optional<Callback>::operator=;

    using optional<Callback>::operator*;
    using optional<Callback>::has_value;
    using optional<Callback>::operator bool;
};

template <typename Callback>
class invoke_on_wrapped_callback : public basic_invoke_on_wrapped_callback<Callback>
{
  public:
    using basic_invoke_on_wrapped_callback<Callback>::basic_invoke_on_wrapped_callback;
    using basic_invoke_on_wrapped_callback<Callback>::operator*;

    template <typename... Args>
    void operator()(Args&&... args) const
    {
        std::invoke(operator*(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    void operator()(Args&&... args)
    {
        std::invoke(operator*(), std::forward<Args>(args)...);
    }
};
} // namespace detail

enum class invoke_on_state : uint8_t
{
    construct = 1 << 0,
    destruct  = 1 << 1,
    copy      = 1 << 2,
    move      = 1 << 3
};

constexpr auto operator&(invoke_on_state left, invoke_on_state right)
{
    return static_cast<uint8_t>(left) & static_cast<uint8_t>(right);
}

constexpr invoke_on_state operator|(invoke_on_state left, invoke_on_state right)
{
    return static_cast<invoke_on_state>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
}

template <invoke_on_state State, typename Callback>
class invoke_on
{
    using raw_callback     = Callback;
    using wrapped_callback = detail::invoke_on_wrapped_callback<raw_callback>;

    static constexpr bool store_wrapped = State & invoke_on_state::move || (State & invoke_on_state::destruct && std::movable<raw_callback>);
    static constexpr bool store_raw     = State & (invoke_on_state::copy | invoke_on_state::destruct);

    using stored_callback_gap = false_type;
    using stored_callback     = conditional_t<store_wrapped, wrapped_callback, conditional_t<store_raw, raw_callback, stored_callback_gap>>;

    [[no_unique_address]] //
    stored_callback callback_;

    template <invoke_on_state CurrentState>
    void do_call()
    {
#ifdef _DEBUG
        if constexpr (!std::same_as<stored_callback, stored_callback_gap>)
#endif
            if constexpr (State & CurrentState)
            {
                if constexpr (CurrentState != invoke_on_state::construct)
                    if constexpr (std::convertible_to<stored_callback, bool>)
                        if (!callback_)
                            return;
                callback_();
            }
    }

  public:
    ~invoke_on()
    {
        do_call<invoke_on_state::destruct>();
    }

    constexpr invoke_on(raw_callback&& callback)
        : callback_{std::move(callback)}
    {
        do_call<invoke_on_state::construct>();
    }

    constexpr invoke_on(raw_callback const& callback)
        : callback_{callback}
    {
        do_call<invoke_on_state::construct>();
    }

    invoke_on(invoke_on const& other)
        : callback_{other.callback_}
    {
        do_call<invoke_on_state::copy>();
    }

    invoke_on& operator=(invoke_on const& other)
    {
        callback_ = other.callback_;
        do_call<invoke_on_state::copy>();
        return *this;
    }

    invoke_on(invoke_on&& other) noexcept
        : callback_{std::move(other.callback_)}
    {
        do_call<invoke_on_state::move>();
    }

    invoke_on& operator=(invoke_on&& other) noexcept
    {
        using std::swap;
        do_call<invoke_on_state::move>();
        other.do_call<invoke_on_state::move>();
        swap(callback_, other.callback_);
        return *this;
    }
};

template <typename Callback>
class invoke_on<invoke_on_state::construct, Callback>
{
  public:
    constexpr invoke_on(Callback&& callback)
    {
        std::invoke(std::move(callback));
    }

    constexpr invoke_on(Callback const& callback)
    {
        std::invoke(callback);
    }
};

template <typename Callback>
using invoke_on_copy = invoke_on<invoke_on_state::copy, Callback>;

template <typename Callback>
using invoke_on_move = invoke_on<invoke_on_state::move, Callback>;

template <typename Callback>
using invoke_on_destruct = invoke_on<invoke_on_state::destruct, Callback>;
} // namespace fd