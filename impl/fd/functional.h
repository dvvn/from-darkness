#pragma once

#include <type_traits>

#ifdef _DEBUG // i dont wanna read shit from std
#undef __cpp_lib_bind_front
#undef __cpp_lib_bind_back
#endif

#if !defined(__cpp_lib_bind_front) || !defined(__cpp_lib_bind_back)
#include <fd/tuple.h>
#endif

#include <function2/function2.hpp>

#include <functional>

namespace fd
{
#if !defined(__cpp_lib_bind_front) || !defined(__cpp_lib_bind_back)
template <uint8_t Mode, typename Fn, class ArgsPacked, typename... Args>
static constexpr decltype(auto) _bind_invoke(Fn&& fn, ArgsPacked& argsPacked, Args&&... args)
{
    return apply(argsPacked, [&]<typename... ArgsUnpacked>(ArgsUnpacked&&... argsUnpacked) {
        if constexpr (Mode == 0)
            return fn(std::forward<ArgsUnpacked>(argsUnpacked)..., std::forward<Args>(args)...);
        else if constexpr (Mode == 1)
            return fn(std::forward<Args>(args)..., std::forward<ArgsUnpacked>(argsUnpacked)...);
    });
}

template <uint8_t Mode, typename... Args>
struct bind_impl : private tuple<Args...>
{
    using tuple<Args...>::tuple;

    template <typename... Args2>
    constexpr decltype(auto) operator()(Args2&&... args2)
    {
        return _bind_invoke<Mode>(this->get(), this->tail(), std::forward<Args2>(args2)...);
    }

    template <typename... Args2>
    constexpr decltype(auto) operator()(Args2&&... args2) const
    {
        return _bind_invoke<Mode>(this->get(), this->tail(), std::forward<Args2>(args2)...);
    }
};
#endif

#ifdef __cpp_lib_bind_front
using std::bind_front;
#else
template <typename... Args>
constexpr bind_impl<0, std::decay_t<Args>...> bind_front(Args&&... args)
{
    return { std::forward<Args>(args)... };
}
#endif

#ifdef __cpp_lib_bind_back
using std::bind_back;
#else
template <typename... Args>
constexpr bind_impl<1, std::decay_t<Args>...> bind_back(Args&&... args)
{
    return { std::forward<Args>(args)... };
}
#endif

template <typename Fn, bool = std::convertible_to<Fn, bool>&& std::assignable_from<Fn, nullptr_t>>
struct lazy_invoke;

template <typename Fn>
class lazy_invoke_base
{
    Fn fn_;

    friend struct lazy_invoke<Fn>;

  public:
    constexpr lazy_invoke_base(Fn fn)
        : fn_(std::move(fn))
    {
    }

    lazy_invoke_base(const lazy_invoke_base&)            = delete;
    lazy_invoke_base& operator=(const lazy_invoke_base&) = delete;
};

template <typename Fn>
struct lazy_invoke<Fn, true> : lazy_invoke_base<Fn>
{
    constexpr ~lazy_invoke()
    {
        if (this->fn_)
            this->fn_();
    }

    using lazy_invoke_base<Fn>::lazy_invoke_base;
    using lazy_invoke_base<Fn>::operator=;

    constexpr void reset()
    {
        this->fn_ = nullptr;
    }
};

template <typename Fn>
struct lazy_invoke<Fn, false> : lazy_invoke_base<Fn>
{
  private:
    bool valid_ = true;

  public:
    constexpr ~lazy_invoke()
    {
        if (valid_)
            this->fn_();
    }

    using lazy_invoke_base<Fn>::lazy_invoke_base;
    using lazy_invoke_base<Fn>::operator=;

    constexpr void reset()
    {
        valid_ = false;
    }
};

template <typename Fn>
lazy_invoke(Fn) -> lazy_invoke<std::decay_t<Fn>>;

using fu2::function;
using fu2::function_view;
using fu2::unique_function;

using fu2::detail::overloading::overload;
using fu2::detail::overloading::overload_impl;
} // namespace fd