#pragma once

#include "basic_callback.h"

#include <functional>
#include <stdexcept>

namespace fd
{
template <size_t I, typename Fn>
struct function_argument;

// template <typename Fn>
// struct function_argument : function_argument<decltype(std::function(std::declval<Fn>()))>
//{
// };

template <bool, size_t I, typename T, typename... Ts>
struct select_argument_impl;

template <size_t I, typename T, typename... Ts>
struct select_argument_impl<true, I, T, Ts...> : select_argument_impl<true, I - 1, Ts...>
{
    using type = T;
};

template <typename T, typename... Ts>
struct select_argument_impl<true, 0, T, Ts...>
{
    using type = T;
};

template <size_t I, typename T, typename... Ts>
struct select_argument_impl<false, I, T, Ts...>
{
    using type = void;
};

template <size_t I, typename... Ts>
using select_argument = typename select_argument_impl<(I < sizeof...(Ts)), I, Ts...>::type;

template <size_t I, typename Ret, typename... Args>
struct function_argument<I, std::function<Ret(Args...)>>
{
    using type = select_argument<I, Args...>;
};

template <typename Fn>
struct function_return;

template <typename Ret, typename... Args>
struct function_return<std::function<Ret(Args...)>>
{
    using type = Ret;
};

template <typename Arg>
class callback_arg_protector final : public boost::noncopyable
{
    Arg arg_;
    Arg backup_;

  public:
    ~callback_arg_protector()
    {
        if (arg_ != backup_)
            throw std::logic_error("Argument changed!");
    }

    callback_arg_protector(Arg const &arg)
        : arg_((arg))
        , backup_(arg)
    {
    }

    callback_arg_protector(Arg &&arg)
        : arg_((arg))
        , backup_(std::move(arg))
    {
    }

    Arg &get()
    {
        return arg_;
    }
};

template <typename Arg>
class callback_arg_protector<Arg &> : public boost::noncopyable
{
    Arg *arg_;
    Arg backup_;

  public:
    ~callback_arg_protector()
    {
        if (*arg_ != backup_)
            throw std::logic_error("Argument changed!");
    }

    callback_arg_protector(Arg &arg)
        : arg_(&arg)
        , backup_(arg)
    {
    }

    Arg &get()
    {
        return static_cast<Arg &>(*arg_);
    }
};

#if 0
template <typename Arg>
class callback_arg_protector<Arg &&> : public callback_arg_protector<Arg>
{
  public:
    callback_arg_protector(Arg &&arg)
        : callback_arg_protector<Arg>(std::move(arg))
    {
    }
};
#endif

template <typename Arg>
callback_arg_protector(Arg) -> callback_arg_protector<Arg &>;
template <typename Arg>
callback_arg_protector(Arg &) -> callback_arg_protector<Arg &>;
template <typename Arg>
callback_arg_protector(Arg *) -> callback_arg_protector<Arg *>;

template <typename Arg, typename Fn>
class callback_function_proxy
{
    Fn fn_;

  public:
    callback_function_proxy(Fn &&fn)
        : fn_(std::move(fn))
    {
    }

    callback_function_proxy(Fn const &fn)
        : fn_((fn))
    {
    }

    decltype(auto) operator()(void *result, callback_stop_token *stop_token)
    {
#ifdef _DEBUG
        if constexpr (!std::is_const_v<Arg> && std::copyable<Arg>)
        {
            auto clone = callback_arg_protector(reinterpret_cast<Arg>(result));
            return std::invoke(fn_, clone.get(), stop_token);
        }
#endif
        return std::invoke(fn_, reinterpret_cast<Arg>(result), stop_token);
    }
};

template <typename Fn>
struct callback_argument : function_argument<0, decltype(std::function(std::declval<Fn>()))>
{
};

template <typename Fn>
struct callback_argument<std::reference_wrapper<Fn>> : callback_argument<Fn>
{
};

template <typename Ret, typename Fn>
class callback final : public basic_callback<Ret>
{
    using function_type = Fn;
    using argument_type = typename callback_argument<Fn>::type;

    callback_function_proxy<argument_type, function_type> fn_;

  public:
    callback(function_type fn)
        : fn_(std::move(fn))
    {
    }

    Ret invoke(void *result, callback_stop_token *stop_token) override
    {
        return fn_(result, stop_token);
    }
};

template <typename Ret, typename Arg, typename Fn>
class callback<Ret, callback_function_proxy<Arg, Fn>> final : public basic_callback<Ret>
{
    callback_function_proxy<Arg, Fn> fn_;

  public:
    callback(std::in_place_type_t<Arg>, Fn fn)
        : fn_(std::move(fn))
    {
    }

    callback(Fn fn)
        : fn_(std::move(fn))
    {
    }

    Ret invoke(void *result, callback_stop_token *stop_token) override
    {
        return fn_(result, stop_token);
    }
};

template <typename Ret, typename Fn>
class callback<Ret, Fn &>;

// template <typename Fn>
// callback(Fn) -> callback<std::decay_t<Fn>>;
//
// template <typename Arg, typename Fn>
// callback(std::in_place_type_t<Arg>, Fn) -> callback<callback_function_proxy<Arg, std::decay_t<Fn>>>;

}