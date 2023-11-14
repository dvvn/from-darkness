#pragma once

#include "functional/call_traits.h"
#include "functional/invoke_cast.h"

#include <boost/hana/tuple.hpp>

#include <concepts>

namespace fd
{
template <class Call_T>
void* get_vfunc(void* table_function, void* instance);

#define GET_VFUNC(call__, __call, _call_) \
    template <>                           \
    void* get_vfunc<call__>(void* table_function, void* instance);

X86_CALL_MEMBER(GET_VFUNC);
#undef GET_VFUNC

template <typename Fn>
void* get_vfunc(Fn table_function, void* instance)
#if 0
    requires(std::is_member_function_pointer_v<Fn>)
#else
    requires requires { typename function_info<Fn>::object_type; }
#endif
{
    using call_type = typename function_info<Fn>::call_type;
    void* function  = unsafe_cast_lazy(table_function);
    return get_vfunc<call_type>(function, instance);
}

template <class Object>
class basic_vfunc
{
    void* function_;
    Object* instance_;

  public:
    basic_vfunc(void* function, Object* instance)
        : function_(function)
        , instance_(instance)
    {
    }

    void* get() const
    {
        return function_;
    }

    operator void*() const
    {
        return function_;
    }

    Object* instance() const
    {
        return instance_;
    }
};

template <typename Object>
class basic_vfunc<Object const>;

template <class Call_T, typename Ret, class Object, typename... Args>
struct vfunc : basic_vfunc<Object>
{
    using function_type = typename member_function<Call_T, Ret, Object, Args...>::type;

    using basic_vfunc<Object>::basic_vfunc;

    vfunc(function_type function, Object* instance)
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    /*function_type get_full() const
    {
        return unsafe_cast<function_type>(basic_vfunc<Object>::get());
    }*/

    /*Ret operator()(Args... args) const
    {
        member_func_invoker<Call_T, Ret, T, Args...> invoker;
        return invoker(function_, instance_, args...);
    }*/
};

template <class Call_T, typename Ret, class Object, typename... Args>
auto get(vfunc<Call_T, Ret, Object, Args...> vf) -> typename decltype(vf)::function_type
{
    return unsafe_cast_lazy(vf.get());
}

template <class Call_T, typename Ret, class Object, typename... Args>
Ret invoke(vfunc<Call_T, Ret, Object, Args...> func, std::type_identity_t<Args>... args)
{
    constexpr member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <class Call_T, typename Ret, typename... Args>
struct vfunc<Call_T, Ret, void, Args...> : basic_vfunc<void>
{
    using basic_vfunc::basic_vfunc;
};

template <class Call_T, typename Ret, class Object, typename... Args>
struct vfunc<Call_T, Ret, Object, function_args<Args...>> : vfunc<Call_T, Ret, Object, Args...>
{
    using vfunc<Call_T, Ret, Object, Args...>::vfunc;
};

#if 0
template <typename Func, class Object>
vfunc(Func fn, Object instance)->typename function_info<Func>::template rebind<vfunc>;
#else
template <typename Func, class Object, class Info = function_info<Func>>
vfunc(Func fn, Object instance) -> vfunc<typename Info::call_type, typename Info::return_type, typename Info::object_type, typename Info::args>;
#endif

template <class Call_T, class Object>
struct unknown_vfunc_args : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc_args(Fn function, Object* instance)
#ifdef _DEBUG
        requires(std::same_as<Call_T, typename function_info<Fn>::call_type>)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    unknown_vfunc_args(ptrdiff_t const function_index, Object* instance)
        : basic_vfunc<Object>(get_vfunc(function_index, instance), instance)
    {
    }

    template <typename Ret, typename... Args>
    auto get() const -> vfunc<Call_T, Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    void* operator+(size_t const offset) const
    {
        return static_cast<uint8_t*>(basic_vfunc<Object>::get()) + offset;
    }
};

namespace detail
{
template <class Call_T, class Object, typename... Args>
class unknown_vfunc_invoker
{
    using args_packed = boost::hana::tuple<Args...>;

    template <typename Ret>
    static constexpr member_func_invoker<Call_T, Ret, Object, Args...> invoker;

    void* function_;
    Object* instance_;
    [[no_unique_address]] //
    args_packed args_;

  public:
    unknown_vfunc_invoker(void* function, Object* instance, Args... args)
        : function_(function)
        , instance_(instance)
        , args_(args...)
    {
    }

    template <typename Ret>
    Ret operator()(std::type_identity<Ret>) const
    {
        return boost::hana::unpack(args_, [this](Args... args) -> Ret {
            return invoker<Ret>(args..., function_, instance_);
        });
    }

    template <typename Ret>
    Ret operator()(std::type_identity<Ret>)
    {
        return boost::hana::unpack(args_, [this](Args... args) -> Ret {
            return invoker<Ret>(args..., function_, instance_);
        });
    }
};
} // namespace detail

template <class Call_T, class Object, typename... Args>
auto invoke(unknown_vfunc_args<Call_T, Object> func, Args... args) -> invoke_cast<detail::unknown_vfunc_invoker<Call_T, Object, Args...>>
{
    return {func.get(), func.instance(), args...};
}

template <typename Ret, class Call_T, class Object, typename... Args>
Ret invoke(unknown_vfunc_args<Call_T, Object> func, Args... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <typename Ret, class Object, typename... Args>
struct unknown_vfunc_call : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc_call(Fn function, Object* instance)
#ifdef _DEBUG
        requires(std::invocable<Fn, Object*, Args...>)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    template <class Call_T>
    auto get(Call_T = {}) const -> vfunc<Call_T, Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }
};

template <class Call_T, typename Ret, class Object, typename... Args>
Ret invoke(unknown_vfunc_call<Ret, Object, Args...> func, std::type_identity_t<Args>... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <class Object>
struct unknown_vfunc : basic_vfunc<Object>
{
    template <typename Fn>
    unknown_vfunc(Fn function, Object* instance)
#ifdef _DEBUG
        requires(std::convertible_to<Object, typename function_info<Fn>::object_type>)
#endif
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    template <class Call_T, typename Ret, typename... Args>
    auto get() const -> vfunc<Call_T, Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    template <typename Ret, typename... Args>
    auto get() const -> unknown_vfunc_call<Ret, Object, Args...>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }

    template <class Call_T>
    auto get() const -> unknown_vfunc_args<Call_T, Object>
    {
        return {basic_vfunc<Object>::get(), basic_vfunc<Object>::instance()};
    }
};

template <class Call_T, typename Ret, class Object, typename... Args>
Ret invoke(unknown_vfunc<Object> func, Args... args)
{
    member_func_invoker<Call_T, Ret, Object, Args...> invoker;
    return invoker(func.get(), func.instance(), args...);
}

template <class Call_T, class Object, typename... Args>
auto invoke(unknown_vfunc<Object> func, Args... args) -> invoke_cast<detail::unknown_vfunc_invoker<Call_T, Object, Args...>>
{
    return {func.get(), func.instance(), args...};
}
} // namespace fd