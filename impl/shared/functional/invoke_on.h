#pragma once
#include "functional/overload.h"
#include "optional.h"

#include <cassert>
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
    bool has_value() const
    {
        return static_cast<bool>(static_cast<overload_t<Callback> const*>(this));
    }

    overload_t<Callback>& operator*()
    {
        assert(has_value());
        return *this;
    }

    overload_t<Callback> const& operator*() const
    {
        assert(has_value());
        return *this;
    }

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

template <typename Callback>
struct nullable_callback<Callback, false> : optional<Callback>
{
    using optional<Callback>::operator*;

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

template <typename, typename... Next>
struct last_arg : last_arg<Next...>
{
};

template <typename T>
struct last_arg<T> : std::type_identity<T>
{
};
} // namespace detail

#define OBJ_STATE(_X_) \
    struct _X_ final   \
    {                  \
    };

struct object_state final
{
    OBJ_STATE(construct);
    OBJ_STATE(destruct);
    OBJ_STATE(copy);
    OBJ_STATE(move);
};

#undef OBJ_STATE

template <typename... T>
class invoke_on
{
    using objs = object_state;

    using raw_callback      = typename detail::last_arg<T...>::type;
    using nullable_callback = detail::nullable_callback<raw_callback>;

    template <class TargetState>
    static constexpr bool have_state = (std::same_as<T, TargetState> || ...);

#ifdef _DEBUG
    static_assert(have_state<objs::construct> || have_state<objs::destruct> || have_state<objs::copy> || have_state<objs::move>);
#endif

    static constexpr bool store_nullable = have_state<objs::move> || (std::movable<raw_callback> && have_state<objs::destruct>);
    static constexpr bool store_raw      = have_state<objs::copy> || have_state<objs::destruct>;

    using stored_callback_gap = std::false_type;
    using stored_callback     = std::conditional_t<store_nullable, nullable_callback, std::conditional_t<store_raw, raw_callback, stored_callback_gap>>;

    [[no_unique_address]] //
    stored_callback callback_;

    template <class CallbackFwd>
    static constexpr decltype(auto) rewrap_callback(CallbackFwd&& callback)
    {
        if constexpr (std::same_as<stored_callback, stored_callback_gap>)
            return stored_callback_gap();
        else
            return std::forward<CallbackFwd>(callback);
    }

    template <class Callback, class... Next>
    static constexpr decltype(auto) extract_callback(Callback&& callback, Next&&... next)
    {
        if constexpr (sizeof...(Next) == 0)
            return std::forward<Callback>(callback);
        else
            return extract_callback(std::forward<Next>(next)...);
    }

    template <class CurrentState>
    void do_call()
    {
        if constexpr (have_state<CurrentState>)
        {
            if constexpr (std::convertible_to<stored_callback, bool> && !std::same_as<CurrentState, objs::construct>)
                if (!callback_)
                    return;
            callback_();
        }
    }

  public:
    ~invoke_on()
    {
        do_call<objs::destruct>();
    }

    template <typename Fn>
    constexpr invoke_on(Fn&& callback) requires(std::constructible_from<stored_callback, Fn &&>)
        : callback_(std::forward<Fn>(callback))
    {
        do_call<objs::construct>();
    }

    /*template <typename Fn>
    constexpr invoke_on(Fn&& callback)
        requires(std::constructible_from<raw_callback, Fn &&> && have_state<objs::construct> && std::same_as<stored_callback, stored_callback_gap>)
    {
        std::invoke(std::forward<Fn>(callback));
    }*/

    template <typename... Args>
    constexpr invoke_on(Args&&... args)
        : invoke_on(extract_callback(std::forward<Args>(args)...))
    {
    }

    invoke_on(invoke_on const& other)
        : callback_(other.callback_)
    {
        do_call<objs::copy>();
    }

    invoke_on& operator=(invoke_on const& other)
    {
        callback_ = other.callback_;
        do_call<objs::copy>();
        return *this;
    }

    invoke_on(invoke_on&& other) noexcept
        : callback_(std::move(other.callback_))
    {
        do_call<objs::move>();
    }

    invoke_on& operator=(invoke_on&& other) noexcept
    {
        using std::swap;
        do_call<objs::move>();
        other.do_call<objs::move>();
        swap(callback_, other.callback_);
        return *this;
    }
};

template <typename Callback>
class invoke_on<Callback>;

template <typename Callback>
class invoke_on<object_state::construct, Callback>
{
  public:
    template <typename Fn>
    constexpr invoke_on(Fn&& callback) requires(std::constructible_from<Callback, Fn &&>)
    {
        std::invoke(std::forward<Fn>(callback));
    }
};

#if 0
template <class... T>
#ifdef _DEBUG
requires(sizeof...(T) != 0 && sizeof...(T) <= 5)
#endif
invoke_on(T&&...) -> invoke_on<std::decay_t<T>...>;
#else
template <class A0, typename Callback>
invoke_on(A0, Callback&&) -> invoke_on<A0, std::decay_t<Callback>>;
template <class A0, class A1, typename Callback>
invoke_on(A0, A1, Callback&&) -> invoke_on<A0, A1, std::decay_t<Callback>>;
template <class A0, class A1, class A2, typename Callback>
invoke_on(A0, A1, A2, Callback&&) -> invoke_on<A0, A1, A2, std::decay_t<Callback>>;
template <class A0, class A1, class A2, class A3, typename Callback>
invoke_on(A0, A1, A2, A3, Callback&&) -> invoke_on<A0, A1, A2, A3, std::decay_t<Callback>>;
#endif
}