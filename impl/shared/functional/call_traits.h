#pragma once

#include "concepts.h"
#include "diagnostics/fatal.h"
#include "functional/bind.h"

#include <x86RetSpoof.h>

#include <cassert>
#include <tuple>

#undef thiscall
#undef cdecl
#undef fastcall
#undef stdcall
#undef vectorcall

namespace fd
{
template <typename To, typename From>
To unsafe_cast(From from)
{
    if constexpr (std::convertible_to<To, From>)
    {
        return static_cast<To>(from);
    }
    else
    {
        static_assert(sizeof(To) == sizeof(From));

        union
        {
            From from0;
            To to;
        };

        from0 = from;
        return to;
    }
}

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

struct dummy_class
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
constexpr call_type_t<C> call_type_v;

template <call_type Call_T, typename Ret, typename T, typename... Args>
requires(std::is_class_v<T> || std::is_union_v<T> /*std::is_fundamental_v<T>*/)
struct member_func_type_impl;

template <call_type Call_T, typename Ret, typename T, typename... Args>
using member_func_type = typename member_func_type_impl<Call_T, Ret, T, Args...>::type;

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
        return std::invoke(unsafe_cast<type>(function), args...);
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
    Ret operator()(member_func_type<Call_T, Ret, T, Args...> function, T *instance, Args... args) const
    {
        return std::invoke(function, instance, args...);
    }

    Ret operator()(void *function, T *instance, Args... args) const
    {
        return try_spoof_member_return_address<Call_T, Ret>(function, instance, args...);
    }
};

template <class Ret, typename... Args>
struct member_func_invoker<call_type::thiscall_, Ret, void, Args...>
    : non_member_func_invoker<call_type::thiscall_, Ret, void *, Args...>
{
};

template <call_type Call_T, class Ret, typename... Args>
struct member_func_invoker<Call_T, Ret, void, Args...>
{
    using type = member_func_type<Call_T, Ret, dummy_class, Args...>;

    Ret operator()(type function, void *instance, Args... args) const
    {
        return std::invoke(function, static_cast<dummy_class *>(instance), args...);
    }

    Ret operator()(void *function, void *instance, Args... args) const
    {
        return operator()(unsafe_cast<type>(function), instance, args...);
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
    template <typename Ret>
    using invoker     = member_func_invoker<Call_T, Ret, T, Args...>;
    using args_packed = std::tuple<Args...>;

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
        return std::apply(bind_front(invoker<Ret>(), function_, instance_), args_);
    }
};

template <typename Fn>
struct function_info;

template <typename Ret, typename T, typename... Args>
struct function_info<Ret(__thiscall *)(T *, Args...)>
{
    static constexpr auto call_type = call_type::thiscall_;
    using return_type               = Ret;
    using self_type                 = T;
};

#define FN_INFO(call__, __call, _call_)                   \
    template <typename Ret, typename T, typename... Args> \
    struct function_info<Ret (__call T::*)(Args...)>      \
    {                                                     \
        static constexpr auto call_type = call__;         \
        using return_type               = Ret;            \
        using self_type                 = T;              \
    };                                                    \
    template <typename Ret, typename... Args>             \
    struct function_info<Ret(__call *)(Args...)>          \
    {                                                     \
        static constexpr auto call_type = call__;         \
        using return_type               = Ret;            \
    };

X86_CALL_MEMBER(FN_INFO)
#undef FN_INFO

template <typename Fn>
concept member_function = requires { typename function_info<Fn>::self_type; };

template <typename Fn>
concept unwrapped_member_function = std::is_void_v<typename function_info<Fn>::self_type>;

template <typename Fn>
concept non_member_function = !member_function<Fn> || unwrapped_member_function<Fn>;
} // namespace fd