#pragma once

#include "basic_callback.h"
#include "core.h"

#include "tool/functional/argument.h"

namespace fd
{

namespace impl
{
template <typename Arg>
class callback_arg_protector final : public noncopyable
{
    Arg arg_;
    Arg backup_;

  public:
    ~callback_arg_protector()
    {
        if (arg_ != backup_)
            std::unreachable();
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
class callback_arg_protector<Arg &> : public noncopyable
{
    Arg *arg_;
    Arg backup_;

  public:
    ~callback_arg_protector()
    {
        if (*arg_ != backup_)
            std::unreachable();
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

template <typename Arg>
class callback_arg_protector<Arg &&>;

template <typename Arg>
callback_arg_protector(Arg) -> callback_arg_protector<Arg &>;
template <typename Arg>
callback_arg_protector(Arg &) -> callback_arg_protector<Arg &>;
template <typename Arg>
callback_arg_protector(Arg *) -> callback_arg_protector<Arg *>;

template <typename Fn>
struct callback_argument : function_argument<0, decltype(std::function(std::declval<Fn>()))>
{
};

template <typename Fn>
struct callback_argument<std::reference_wrapper<Fn>> : callback_argument<Fn>
{
};
} // namespace impl

template <typename Arg, typename Fn>
class callback_function_proxy
{
    Fn fn_;

    static Arg do_cast(void *&result)
    {
        if constexpr (std::convertible_to<void *, Arg>)
            return static_cast<Arg>(result);
        else
            return reinterpret_cast<Arg>(result);
    }

  public:
    callback_function_proxy(Fn &&fn)
        : fn_(std::move(fn))
    {
    }

    callback_function_proxy(Fn const &fn)
        : fn_((fn))
    {
    }

    template <typename Ret, typename... Args>
    Ret operator()(std::in_place_type_t<Ret>, void *result, Args... args)
    {
        if constexpr (std::invocable<Fn, Arg, Args...>)
        {
#ifdef _DEBUG
            if constexpr (!std::is_const_v<Arg> && std::copyable<Arg>)
            {
                auto clone = callback_arg_protector(do_cast(result));
                return std::invoke(fn_, clone.get(), args...);
            }
#endif
            return std::invoke(fn_, do_cast(result), args...);
        }

        std::unreachable();
    }
};

template <typename Ret, typename Fn>
struct callback final : basic_callback<Ret>
{
    using basic_callback<Ret>::return_type;
    using function_type = Fn;
    using argument_type = typename impl::callback_argument<Fn>::type;

  private:
    callback_function_proxy<argument_type, function_type> proxy_;

  public:
    callback(function_type fn)
        : proxy_(std::move(fn))
    {
    }

    bool have_stop_token() const override
    {
        return std::invocable<Fn, argument_type, callback_stop_token *>;
    }

    return_type invoke(void *result, callback_stop_token *stop_token) override
    {
        return proxy_(std::in_place_type<return_type>, result, stop_token);
    }

    Ret invoke(void *result) override
    {
        return proxy_(std::in_place_type<Ret>, result);
    }
};

template <typename Ret, typename Arg, typename Fn>
struct callback<Ret, callback_function_proxy<Arg, Fn>> final : basic_callback<Ret>
{
    using basic_callback<Ret>::return_type;

  private:
    callback_function_proxy<Arg, Fn> proxy_;

  public:
    callback(std::in_place_type_t<Arg>, Fn fn)
        : proxy_(std::move(fn))
    {
    }

    callback(Fn fn)
        : proxy_(std::move(fn))
    {
    }

    bool have_stop_token() const override
    {
        return std::invocable<Fn, Arg, callback_stop_token *>;
    }

    Ret invoke(void *result, callback_stop_token *stop_token) override
    {
        return proxy_(std::in_place_type<return_type>, result, stop_token);
    }

    Ret invoke(void *result) override
    {
        return proxy_(std::in_place_type<Ret>, result);
    }
};

template <typename Ret, typename Fn>
struct callback<Ret, Fn &>;

// template <typename Fn>
// callback(Fn) -> callback<std::decay_t<Fn>>;
//
// template <typename Arg, typename Fn>
// callback(std::in_place_type_t<Arg>, Fn) -> callback<callback_function_proxy<Arg, std::decay_t<Fn>>>;

} // namespace fd