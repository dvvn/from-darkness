#pragma once

#include "core.h"
#include "tool/functional.h"

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
enum class call_type_t : uint8_t
{
    // ReSharper disable CppInconsistentNaming
    thiscall_,
    cdecl_,
    fastcall_,
    stdcall_,
    vectorcall_,
    // ReSharper restore CppInconsistentNaming
    // unknown,
};

#if 0
#define X86_CALL_PROXY(call__, __call, call)
#define X86_CALL_PROXY_MEMBER(call__, __call, call)
#endif

#define _X86_CALL_PROXY(_PROXY_, _T_) _PROXY_(call_type_t::_T_##_, __##_T_, ##_T_)

#define X86_CALL(_PROXY_)              \
    _X86_CALL_PROXY(_PROXY_, cdecl)    \
    _X86_CALL_PROXY(_PROXY_, fastcall) \
    _X86_CALL_PROXY(_PROXY_, stdcall)  \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER(_PROXY_)       \
    _X86_CALL_PROXY(_PROXY_, thiscall) \
    X86_CALL(_PROXY_)

template <template <call_type_t Call_T> class Q, typename... Args>
decltype(auto) apply(call_type_t info, Args... args)
{
#define INFO_CASE(call__, __call, _call_) \
    case call__:                          \
        return std::invoke(Q<call__>(), args...);

    switch (info)
    {
        X86_CALL_MEMBER(INFO_CASE);
    default:
        std::unreachable();
    }

#undef INFO_CASE
}

#if 0
#define X86_CALL_TYPE(call__, __call, call)                                      \
    template <typename Ret, typename T, typename... Args>                        \
    constexpr call_type_t get_call_type(Ret (__call T::*)(Args...))              \
    {                                                                            \
        return call__;                                                           \
    }                                                                            \
    template <typename Ret, typename T, typename... Args>                        \
    constexpr call_type_t get_call_type_member(Ret (__call T::*)(Args...))       \
    {                                                                            \
        return call__;                                                           \
    }                                                                            \
    template <typename Ret, typename T, typename... Args>                        \
    constexpr call_type_t get_call_type(Ret (__call T::*)(Args...) const)        \
    {                                                                            \
        return call__;                                                           \
    }                                                                            \
    template <typename Ret, typename T, typename... Args>                        \
    constexpr call_type_t get_call_type_member(Ret (__call T::*)(Args...) const) \
    {                                                                            \
        return call__;                                                           \
    }                                                                            \
    template <typename Ret, typename... Args>                                    \
    constexpr call_type_t get_call_type(Ret(__call *)(Args...))                  \
    {                                                                            \
        return call__;                                                           \
    }

X86_CALL_MEMBER(X86_CALL_TYPE);
#undef X86_CALL_TYPE

template <typename T>
concept member_function = requires(T fn) { get_call_type_member(fn); };

template <typename Fn, call_type_t Call_T>
concept same_call_type = get_call_type(Fn()) == Call_T;
#endif

template <call_type_t C>
struct call_type_holder
{
    static constexpr call_type_t value = C;

    constexpr operator call_type_t() const
    {
        return C;
    }
};

namespace call_type
{
#define X86_CALL_TYPE(call__, __call, call) constexpr call_type_holder<call__> call##_;
X86_CALL_MEMBER(X86_CALL_TYPE)
#undef X86_CALL_TYPE
} // namespace call_type

template <typename Fn>
Fn void_to_func(void *function)
{
    if constexpr (std::convertible_to<void *, Fn>)
        return static_cast<Fn>(function);
    else
    {
        static_assert(sizeof(Fn) == sizeof(void *));

        union
        {
            void *from;
            Fn to;
        };

        from = function;
        return to;
    }
}

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
requires(std::is_class_v<T> || std::is_union_v<T> /*std::is_fundamental_v<T>*/)
struct member_func_type_impl;

template <call_type_t Call_T, typename Ret, typename T, typename... Args>
using member_func_type = typename member_func_type_impl<Call_T, Ret, T, Args...>::type;

template <class Object>
struct return_address_gadget;

template <class Object>
concept valid_return_address_gadget = requires { return_address_gadget<Object>::address; };

template <call_type_t Call_T, typename Ret, typename... Args>
struct return_address_spoofer;

template <call_type_t Call_T, typename Ret, class Object, typename... Args>
decltype(auto) try_spoof_member_return_address(void *function, Object *instance, Args... args)
{
    using spoofer = return_address_spoofer<Call_T, Ret, Object *, Args...>;
    using gadget  = return_address_gadget<Object>;

    constexpr auto can_spoof = valid_return_address_gadget<gadget> &&
                               std::invocable<spoofer, uintptr_t, void *, Object *, Args...>;

    if constexpr (can_spoof)
        return std::invoke(spoofer(), gadget::address, function, instance, args...);
    else
        return std::invoke(void_to_func<member_func_type<Call_T, Ret, Object, Args...>>(function), instance, args...);
}

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type_t::cdecl_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void *function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeCdecl<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type_t::stdcall_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void *function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeStdcall<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, class Object, typename... Args>
struct return_address_spoofer<call_type_t::thiscall_, Ret, Object *, Args...>
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

template <call_type_t Call_T, typename Ret, typename... Args>
struct non_member_func_type_impl;

template <call_type_t Call_T, typename Ret, typename... Args>
using non_member_func_type = typename non_member_func_type_impl<Call_T, Ret, Args...>::type;

template <call_type_t Call_T, class Ret, typename... Args>
struct non_member_func_invoker
{
    using type = non_member_func_type<Call_T, Ret, Args...>;

    Ret operator()(type function, Args... args) const
    {
        return std::invoke(function, args...);
    }

    Ret operator()(void *function, Args... args) const
    {
        return std::invoke(void_to_func<type>(function), args...);
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

template <call_type_t Call_T, class Ret, class T, typename... Args>
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
struct member_func_invoker<call_type_t::thiscall_, Ret, void, Args...>
    : non_member_func_invoker<call_type_t::thiscall_, Ret, void *, Args...>
{
};

template <call_type_t Call_T, class Ret, typename... Args>
struct member_func_invoker<Call_T, Ret, void, Args...>
{
    struct dummy_class
    {
    };

    using type = member_func_type<Call_T, Ret, dummy_class, Args...>;

    Ret operator()(type function, void *instance, Args... args) const
    {
        return std::invoke(function, static_cast<dummy_class *>(instance), args...);
    }

    Ret operator()(void *function, void *instance, Args... args) const
    {
        return operator()(void_to_func<type>(function), instance, args...);
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

template <call_type_t Call_T, typename T, typename... Args>
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
    static constexpr auto call_type = call_type_t::thiscall_;
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