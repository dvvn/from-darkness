#pragma once

// #define FD_SPOOF_RETURN_ADDRESS

#include "concepts.h"
#include "diagnostics/fatal.h"
#include "functional/cast.h"

#ifdef FD_SPOOF_RETURN_ADDRESS
#include <x86RetSpoof.h>
#endif

#include <boost/hana/tuple.hpp>

#undef thiscall
#undef cdecl
#undef fastcall
#undef stdcall
#undef vectorcall

namespace fd
{
enum class call_type : uint8_t
{
    // ReSharper disable CppInconsistentNaming
    thiscall_,
    cdecl_,
    fastcall_,
    stdcall_,
    vectorcall_,
    // ReSharper restore CppInconsistentNaming
};

template <call_type C>
struct call_type_t
{
    static constexpr call_type value = C;

    constexpr operator call_type() const
    {
        return C;
    }
};

template <call_type C>
inline constexpr call_type_t<C> call_type_v;

struct dummy_class final
{
};

#if 0
#define X86_CALL_PROXY(call__, __call, call)
#define X86_CALL_PROXY_MEMBER(call__, __call, call)
#endif

#define _X86_CALL_PROXY(_PROXY_, _T_) _PROXY_(call_type::_T_##_, __##_T_, ##_T_)

#define X86_CALL(_PROXY_)              \
    _X86_CALL_PROXY(_PROXY_, cdecl)    \
    _X86_CALL_PROXY(_PROXY_, fastcall) \
    _X86_CALL_PROXY(_PROXY_, stdcall)  \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER(_PROXY_)       \
    _X86_CALL_PROXY(_PROXY_, thiscall) \
    X86_CALL(_PROXY_)

template <template <call_type Call_T> class Q, typename... Args>
decltype(auto) apply(call_type info, Args... args)
{
#define INFO_CASE(call__, __call, _call_) \
    case call__:                          \
        return std::invoke(Q<call__>(), args...);

    switch (info)
    {
        X86_CALL_MEMBER(INFO_CASE);
    default:
        unreachable();
    }

#undef INFO_CASE
}

template <typename Fn, typename... Args>
decltype(auto) apply(Fn fn, call_type info, Args... args)
{
#define INFO_CASE(call__, __call, _call_) \
    case call__:                          \
        return fn(call_type_v<call__>, args...);

    switch (info)
    {
        X86_CALL_MEMBER(INFO_CASE);
    default:
        unreachable();
    }

#undef INFO_CASE
}

template <call_type Call_T, typename Ret, typename T, typename... Args>
requires(std::is_class_v<T> || std::is_union_v<T> /*std::is_fundamental_v<T>*/)
struct member_func_type_impl;

template <call_type Call_T, typename Ret, typename T, typename... Args>
using member_func_type = typename member_func_type_impl<Call_T, Ret, T, Args...>::type;

#ifdef FD_SPOOF_RETURN_ADDRESS
template <class Object>
struct return_address_gadget;

template <class Object>
concept valid_return_address_gadget = requires { return_address_gadget<Object>::address; };

template <call_type Call_T, typename Ret, typename... Args>
struct return_address_spoofer;

template <call_type Call_T, typename Ret, class Object, typename... Args>
decltype(auto) try_spoof_member_return_address(void *function, Object *instance, Args... args)
{
    using spoofer = return_address_spoofer<Call_T, Ret, Object *, Args...>;

    constexpr auto can_spoof  = valid_return_address_gadget<Object>;
    constexpr auto can_invoke = std::invocable<spoofer, uintptr_t, void *, Object *, Args...>;

    if constexpr (can_spoof && can_invoke)
    {
        using gadget = return_address_gadget<Object>;
        return std::invoke(spoofer(), gadget::address, function, instance, args...);
    }
    else
    {
        using obj_t = std::conditional_t<forwarded<Object>, dummy_class, Object>;
        using fn_t  = member_func_type<Call_T, Ret, obj_t, Args...>;
        return std::invoke(unsafe_cast<fn_t>(function), unsafe_cast<obj_t *>(instance), args...);
    }
}

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type::cdecl_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void *function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeCdecl<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type::stdcall_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void *function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeStdcall<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, class Object, typename... Args>
struct return_address_spoofer<call_type::thiscall_, Ret, Object *, Args...>
{
    Ret operator()(uintptr_t gadget, void *function, Object *instance, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeThiscall<Ret, Args...>(
            reinterpret_cast<uintptr_t>(instance), reinterpret_cast<uintptr_t>(function), gadget, args...);
    }

    Ret operator()(void *function, Object *instance, Args... args) const requires(valid_return_address_gadget<Object>)
    {
        return operator()(return_address_gadget<Object>::address, function, instance, args...);
    }
};
#endif

template <call_type Call_T, typename Ret, typename... Args>
struct non_member_func_type_impl;

template <call_type Call_T, typename Ret, typename... Args>
using non_member_func_type = typename non_member_func_type_impl<Call_T, Ret, Args...>::type;

template <call_type Call_T, class Ret, typename... Args>
struct non_member_func_invoker
{
    using type = non_member_func_type<Call_T, Ret, Args...>;

    Ret operator()(type function, Args... args) const
    {
        return std::invoke(function, args...);
    }

    Ret operator()(void *function, Args... args) const
    {
        return operator()(unsafe_cast<type>(function), args...);
    }
};

#define MEMBER_FN_TYPE(call__, __call, _call_)            \
    template <typename Ret, typename T, typename... Args> \
    struct member_func_type_impl<call__, Ret, T, Args...> \
    {                                                     \
        using type = Ret (__call T::*)(Args...);          \
    };

X86_CALL_MEMBER(MEMBER_FN_TYPE)
#undef MEMBER_FN_BUILDER

template <call_type Call_T, class Ret, class T, typename... Args>
struct member_func_invoker
{
    using function_type = member_func_type<Call_T, Ret, T, Args...>;

    Ret operator()(function_type function, T *instance, Args... args) const
    {
        return std::invoke(function, instance, args...);
    }

    Ret operator()(void *function, T *instance, Args... args) const
    {
#ifdef FD_SPOOF_RETURN_ADDRESS
        return try_spoof_member_return_address<Call_T, Ret>(function, instance, args...);
#else
        return operator()(unsafe_cast<function_type>(function), instance, args...);
#endif
    }
};

template <call_type Call_T, class Ret, class T, typename... Args>
requires(std::is_class_v<T> && !complete<T>)
struct member_func_invoker<Call_T, Ret, T, Args...> : member_func_invoker<Call_T, Ret, void, Args...>
{
    static_assert(sizeof(member_func_type<Call_T, Ret, T, Args...>) != sizeof(void *));
};

template <class Ret, typename... Args>
struct member_func_invoker<call_type::thiscall_, Ret, void, Args...>
    : non_member_func_invoker<call_type::thiscall_, Ret, void *, Args...>
{
};

template <call_type Call_T, class Ret, typename... Args>
struct member_func_invoker<Call_T, Ret, void, Args...>
{
    using function_type = member_func_type<Call_T, Ret, dummy_class, Args...>;

    Ret operator()(function_type function, void *instance, Args... args) const
    {
        return std::invoke(function, static_cast<dummy_class *>(instance), args...);
    }

    Ret operator()(void *function, void *instance, Args... args) const
    {
        return operator()(unsafe_cast<function_type>(function), instance, args...);
    }
};

#define NON_MEMBER_FN_TYPE(call__, __call, _call_)         \
    template <typename Ret, typename... Args>              \
    struct non_member_func_type_impl<call__, Ret, Args...> \
    {                                                      \
        using type = Ret(__call *)(Args...);               \
    };

X86_CALL_MEMBER(NON_MEMBER_FN_TYPE)
#undef NON_MEMBER_FN_TYPE

template <call_type Call_T, typename T, typename... Args>
class member_func_return_type_resolver
{
    using args_packed = boost::hana::tuple<Args...>;

    void *function_;
    T *instance_;
    [[no_unique_address]] //
    args_packed args_;

  public:
    member_func_return_type_resolver(void *function, T *instance, Args... args)
        : function_(function)
        , instance_(instance)
        , args_(args...)
    {
    }

    template <typename Ret>
    operator Ret()
    {
        return boost::hana::unpack(args_, [this](Args... args) {
            member_func_invoker<Call_T, Ret, T, Args...> invoker;
            return invoker(args..., function_, instance_);
        });
    }
};

template <typename... T>
struct function_args
{
};

namespace detail
{
template <size_t Target, size_t Current, typename, typename... T>
struct function_arg : function_arg<Target, Current + 1, T...>
{
};

template <size_t Target, typename Arg, typename... T>
struct function_arg<Target, Target, Arg, T...>
{
    using type = Arg;
};

template <size_t Target, size_t Current, typename... T>
struct function_arg<Target, Current, function_args<T...>> : function_arg<Target, Current, T...>
{
};

template <size_t Target, typename... T>
using function_arg_t = typename function_arg<Target, 0, T...>::type;
} // namespace detail

template <call_type Call_T, typename Ret>
struct basic_function_info
{
    static constexpr call_type_t<Call_T> call;
    using return_type = Ret;
};

template <typename... Args>
struct function_info_args
{
    using args = function_args<Args...>;
    template <size_t I>
    using arg = detail::function_arg_t<I, Args...>;
};

template <>
struct function_info_args<>
{
    using args = function_args<>;
    template <size_t I>
    using arg = void;
};

template <call_type Call_T, typename Ret, typename T, typename... Args>
struct member_function_info : basic_function_info<Call_T, Ret>, function_info_args<Args...>
{
    using self_type = T;
};

template <call_type Call_T, typename Ret, typename... Args>
struct non_member_function_info : basic_function_info<Call_T, Ret>, function_info_args<Args...>
{
};

template <typename Fn>
struct function_info;

template <typename Obj>
requires requires { &Obj::operator(); }
struct function_info<Obj> : function_info<decltype(&Obj::operator())>
{
};

template <typename Ret, typename T, typename... Args>
struct function_info<Ret(__thiscall *)(T *, Args...)> : member_function_info<call_type::thiscall_, Ret, T, Args...>
{
};

// template <typename Ret, typename T, typename... Args>
// struct non_member_function_info<call_type::thiscall_, Ret, T, Args...>
//     : member_function_info<call_type::thiscall_, Ret, T, Args...>
//{
// };

#define FN_INFO_MEMBER(call__, __call, _call_)                                                                   \
    template <typename Ret, typename T, typename... Args>                                                        \
    struct function_info<Ret (__call T::*)(Args...)> : member_function_info<call__, Ret, T, Args...>             \
    {                                                                                                            \
    };                                                                                                           \
    template <typename Ret, typename T, typename... Args>                                                        \
    struct function_info<Ret (__call T::*)(Args...) const> : member_function_info<call__, Ret, T const, Args...> \
    {                                                                                                            \
    };

X86_CALL_MEMBER(FN_INFO_MEMBER)
#undef FN_INFO_MEMBER

#define FN_INFO_NON_MEMEBER(call__, __call, _call_)                                               \
    template <typename Ret, typename... Args>                                                     \
    struct function_info<Ret(__call *)(Args...)> : non_member_function_info<call__, Ret, Args...> \
    {                                                                                             \
    };
X86_CALL(FN_INFO_NON_MEMEBER)
#undef FN_INFO_NON_MEMEBER

template <typename Fn>
concept member_function = requires { typename function_info<Fn>::self_type; };

template <typename Fn>
concept unwrapped_member_function = std::is_void_v<typename function_info<Fn>::self_type>;

template <typename Fn>
concept non_member_function = !member_function<Fn> || unwrapped_member_function<Fn>;
} // namespace fd