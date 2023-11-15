#pragma once

#include "functional/call_traits.h"
#include "functional/invoke_cast.h"

#include <boost/hana/tuple.hpp>

#include <concepts>

namespace fd
{
template <class Call_T>
void* get_vfunc(void* table_function, void* instance);

#define GET_VFUNC(_CCV_, ...) \
    template <>               \
    void* get_vfunc<_CCV_T(_CCV_)>(void* table_function, void* instance);

#ifdef _MSC_VER
_MEMBER_CALL(GET_VFUNC, , , )
#else

#endif
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
    auto function   = unsafe_cast<void*>(table_function);
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
    template <bool Noexcept>
    using function_type = typename member_function<Noexcept, Call_T, Ret, Object, Args...>::type;

    using basic_vfunc<Object>::basic_vfunc;

    vfunc(function_type<true> function, Object* instance)
        : basic_vfunc<Object>(get_vfunc(function, instance), instance)
    {
    }

    vfunc(function_type<false> function, Object* instance)
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
auto get(vfunc<Call_T, Ret, Object, Args...> vf) -> typename decltype(vf)::template function_type<false>
{
    return unsafe_cast_lazy(vf.get());
}

template <class Call_T, typename Ret, class Object, typename... Args>
Ret invoke(vfunc<Call_T, Ret, Object, Args...> func, std::type_identity_t<Args>... args)
{
#ifdef FD_SPOOF_RETURN_ADDRESS
    auto instance = safe_cast<void>(func.instance());
#else
    auto instance = func.instance();
#endif
    return invoke(get(func), instance, args...);
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
    using raw_function = member_function<false, Call_T, Ret, Object, Args...>;

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
            return invoke(unsafe_cast<raw_function<Ret>>(function_), instance_, args...);
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
    return invoke(func.template get<Ret, Args...>(), args...);
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
    return invoke(func.template get<Call_T>(), args...);
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
    return invoke(func.template get<Call_T, Ret, Args...>(), args...);
}

template <class Call_T, class Object, typename... Args>
auto invoke(unknown_vfunc<Object> func, Args... args) -> invoke_cast<detail::unknown_vfunc_invoker<Call_T, Object, Args...>>
{
    return {func.get(), func.instance(), args...};
}
} // namespace fd