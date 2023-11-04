#pragma once

// #define FD_SPOOF_RETURN_ADDRESS

#include "concepts.h"
#include "diagnostics/fatal.h"
#include "functional/cast.h"

#ifdef FD_SPOOF_RETURN_ADDRESS
#include <x86RetSpoof.h>
#endif

#include <boost/hana/tuple.hpp>

#include <functional>

#undef thiscall
#undef cdecl
#undef fastcall
#undef stdcall
#undef vectorcall

namespace fd
{
#define CALL_T_ITEM(_V_) \
    struct _V_##_ final  \
    {                    \
    };

struct call_type final
{
#if INTPTR_MAX == INT32_MAX
    CALL_T_ITEM(thiscall);
    CALL_T_ITEM(fastcall);
    CALL_T_ITEM(stdcall);
#endif
    CALL_T_ITEM(cdecl);
    CALL_T_ITEM(vectorcall);
};

#undef CALL_T_ITEM

#if 0
#define X86_CALL_PROXY(call__, __call, call)
#define X86_CALL_PROXY_MEMBER(call__, __call, call)
#endif

#define _X86_CALL_PROXY(_PROXY_, _T_) _PROXY_(call_type::_T_##_, __##_T_, ##_T_)

#if INTPTR_MAX == INT32_MAX
#define X86_CALL(_PROXY_)              \
    _X86_CALL_PROXY(_PROXY_, cdecl)    \
    _X86_CALL_PROXY(_PROXY_, fastcall) \
    _X86_CALL_PROXY(_PROXY_, stdcall)  \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER(_PROXY_)       \
    _X86_CALL_PROXY(_PROXY_, thiscall) \
    X86_CALL(_PROXY_)
#elif INTPTR_MAX == INT64_MAX
#define X86_CALL(_PROXY_)           \
    _X86_CALL_PROXY(_PROXY_, cdecl) \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER X86_CALL
#endif

#if 0
template <template <class Call_T> class Q, typename... Args>
decltype(auto) apply(call_type const info, Args... args)
{
#define INFO_CASE(call__, __call, _call_) \
    case call__:                          \
        return std::invoke(Q<call__>(), args...);

    switch (info)
    {
        X86_CALL_MEMBER(INFO_CASE);
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        unreachable();
    }

#undef INFO_CASE
}

template <typename Fn, typename... Args>
decltype(auto) apply(Fn fn, call_type const info, Args... args)
{
#define INFO_CASE(call__, __call, _call_) \
    case call__:                          \
        return fn(call_type_v<call__>, args...);

    switch (info)
    {
        X86_CALL_MEMBER(INFO_CASE);
    default: // NOLINT(clang-diagnostic-covered-switch-default)
        unreachable();
    }

#undef INFO_CASE
}
#endif

template <class Call_T, typename Ret, typename Object, typename... Args>
requires(std::is_class_v<Object> || std::is_union_v<Object> /*std::is_fundamental_v<T>*/)
struct member_function;

template <class Call_T, typename Ret, typename Object, typename... Args>
using member_function_t = typename member_function<Call_T, Ret, Object, Args...>::type;

#define MEMBER_FN_TYPE(call__, __call, _call_)                                                                             \
    template <typename Ret, typename Object, typename... Args>                                                             \
    struct member_function<call__, Ret, Object, Args...> : std::type_identity<Ret (__call Object::*)(Args...)>             \
    {                                                                                                                      \
    };                                                                                                                     \
    template <typename Ret, typename Object, typename... Args>                                                             \
    struct member_function<call__, Ret, Object const, Args...> : std::type_identity<Ret (__call Object::*)(Args...) const> \
    {                                                                                                                      \
    };

X86_CALL_MEMBER(MEMBER_FN_TYPE)
#undef MEMBER_FN_TYPE

#ifdef FD_SPOOF_RETURN_ADDRESS
template <class Object>
struct return_address_gadget;

template <class Object>
concept valid_return_address_gadget = requires { return_address_gadget<Object>::address; };

template <class Call_T, typename Ret, typename... Args>
struct return_address_spoofer;

template <class Call_T, typename Ret, class Object, typename... Args>
decltype(auto) try_spoof_member_return_address(void* function, Object* instance, Args... args)
{
    using spoofer = return_address_spoofer<Call_T, Ret, Object*, Args...>;

    constexpr auto can_spoof  = valid_return_address_gadget<Object>;
    constexpr auto can_invoke = std::invocable<spoofer, uintptr_t, void*, Object*, Args...>;

    if constexpr (can_spoof && can_invoke)
    {
        using gadget = return_address_gadget<Object>;
        return std::invoke(spoofer(), gadget::address, function, instance, args...);
    }
    else
    {
        using obj_t = std::conditional_t<forwarded<Object>, dummy_class, Object>;
        using fn_t  = member_function_t<Call_T, Ret, obj_t, Args...>;
        return std::invoke(unsafe_cast<fn_t>(function), unsafe_cast<obj_t*>(instance), args...);
    }
}

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type::cdecl_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void* function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeCdecl<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, typename... Args>
struct return_address_spoofer<call_type::stdcall_, Ret, Args...>
{
    Ret operator()(uintptr_t gadget, void* function, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeStdcall<Ret, Args...>(reinterpret_cast<uintptr_t>(function), gadget, args...);
    }
};

template <typename Ret, class Object, typename... Args>
struct return_address_spoofer<call_type::thiscall_, Ret, Object*, Args...>
{
    Ret operator()(uintptr_t gadget, void* function, Object* instance, Args... args) const
    {
        assert(gadget != 0);
        return x86RetSpoof::invokeThiscall<Ret, Args...>(reinterpret_cast<uintptr_t>(instance), reinterpret_cast<uintptr_t>(function), gadget, args...);
    }

    Ret operator()(void* function, Object* instance, Args... args) const requires(valid_return_address_gadget<Object>)
    {
        return operator()(return_address_gadget<Object>::address, function, instance, args...);
    }
};
#endif

template <class Call_T, typename Ret, typename... Args>
struct non_member_function;

template <class Call_T, typename Ret, typename... Args>
using non_member_function_t = typename non_member_function<Call_T, Ret, Args...>::type;

template <class Call_T, class Ret, typename... Args>
struct non_member_func_invoker
{
    using function_type = non_member_function_t<Call_T, Ret, Args...>;

    Ret operator()(function_type function, Args... args) const
    {
        return std::invoke(function, args...);
    }

    Ret operator()(void* function, Args... args) const
    {
        return operator()(unsafe_cast<function_type>(function), args...);
    }
};

template <class Call_T, class Ret, class T, typename... Args>
struct member_func_invoker
{
    using function_type = member_function_t<Call_T, Ret, T, Args...>;

    Ret operator()(function_type function, T* instance, Args... args) const
    {
        return std::invoke(function, instance, args...);
    }

    Ret operator()(void* function, T* instance, Args... args) const
    {
#ifdef FD_SPOOF_RETURN_ADDRESS
        return try_spoof_member_return_address<Call_T, Ret>(function, instance, args...);
#else
        return operator()(unsafe_cast<function_type>(function), instance, args...);
#endif
    }
};

template <class Call_T, class Ret, class T, typename... Args>
requires(std::is_class_v<T> && !complete<T>)
struct member_func_invoker<Call_T, Ret, T, Args...> : member_func_invoker<Call_T, Ret, void, Args...>
{
    static_assert(sizeof(member_function_t<Call_T, Ret, T, Args...>) != sizeof(void*));
};

#if INTPTR_MAX == INT32_MAX
template <class Ret, typename... Args>
struct member_func_invoker<call_type::thiscall_, Ret, void, Args...> : non_member_func_invoker<call_type::thiscall_, Ret, void*, Args...>
{
};
#endif

namespace detail
{
struct dummy_class final
{
};
} // namespace detail

template <class Call_T, class Ret, typename... Args>
struct member_func_invoker<Call_T, Ret, void, Args...>
{
    using function_type = member_function_t<Call_T, Ret, detail::dummy_class, Args...>;

    Ret operator()(function_type function, void* instance, Args... args) const
    {
        return std::invoke(function, static_cast<detail::dummy_class*>(instance), args...);
    }

    Ret operator()(void* function, void* instance, Args... args) const
    {
        return operator()(unsafe_cast<function_type>(function), instance, args...);
    }
};

#define NON_MEMBER_FN_TYPE(call__, __call, _call_)                                               \
    template <typename Ret, typename... Args>                                                    \
    struct non_member_function<call__, Ret, Args...> : std::type_identity<Ret(__call*)(Args...)> \
    {                                                                                            \
    };

X86_CALL_MEMBER(NON_MEMBER_FN_TYPE)
#undef NON_MEMBER_FN_TYPE

template <class Call_T, class Object, typename... Args>
class member_func_return_type_resolver
{
    using args_packed = boost::hana::tuple<Args...>;

    template <typename Ret>
    using func_invoker = member_func_invoker<Call_T, Ret, Object, Args...>;

    void* function_;
    Object* instance_;
    [[no_unique_address]] //
    args_packed args_;

  public:
    member_func_return_type_resolver(void* function, Object* instance, Args... args)
        : function_(function)
        , instance_(instance)
        , args_(args...)
    {
    }

    /*template <typename Ret>
    operator Ret()
    {
        return boost::hana::unpack(args_, [this](Args... args) {
            func_invoker<Ret> invoker;
            return invoker(args..., function_, instance_);
        });
    }*/

    template <typename Ret>
    operator Ret() const
    {
        return boost::hana::unpack(args_, [this](Args... args) {
            func_invoker<Ret> invoker;
            return invoker(args..., function_, instance_);
        });
    }
};

namespace detail
{
template <size_t Index, typename, typename... T>
struct function_arg : function_arg<Index - 1, T...>
{
};

template <typename T, typename... Next>
struct function_arg<0, T, Next...> : std::type_identity<T>
{
};
} // namespace detail

template <typename... T>
struct function_args
{
    static constexpr size_t count = sizeof...(T);
    template <size_t I>
    using get = typename detail::function_arg<I, T...>::type;
};

template <>
struct function_args<>
{
    static constexpr size_t count = 0;
    template <size_t I>
    using get = void;
};

template <class Call_T, typename Ret>
struct basic_function_info
{
    using call_type   = Call_T;
    using return_type = Ret;
};

template <class Call_T, typename Ret, class T, typename... Args>
struct member_function_info : basic_function_info<Call_T, Ret>
{
    template <template <class, typename, class, typename...> class Other>
    using rebind = Other<Call_T, Ret, T, Args...>;

    using object_type = T;
    using args        = function_args<Args...>;
};

template <class Call_T, typename Ret, typename... Args>
struct non_member_function_info : basic_function_info<Call_T, Ret>
{
    template <template <class, typename, typename...> class Other>
    using rebind = Other<Call_T, Ret, Args...>;

    using args = function_args<Args...>;
};

template <typename Fn>
struct function_info;

// template <typename Fn>
// using function_info_base = typename function_info<Fn>::base;

#ifdef _MSC_VER
template <typename Ret, typename Fn, typename... Args>
struct function_info<std::_Binder<Ret, Fn, Args...>>;

template <typename Fn, typename... Args>
struct function_info<std::_Binder<std::_Unforced, Fn, Args...>> : function_info<Fn>
{
};

#ifdef __cpp_lib_bind_front
template <typename Fn, typename... Args>
struct function_info<std::_Front_binder<Fn, Args...>> : function_info<Fn>
{
};
#endif

#ifdef __cpp_lib_bind_back
template <typename Fn, typename... Args>
struct function_info<std::_Back_binder<Fn, Args...>> : function_info<Fn>
{
};
#endif
#endif

template <typename Obj>
requires requires { &Obj::operator(); }
struct function_info<Obj> : function_info<decltype(&Obj::operator())>
{
};

#if INTPTR_MAX == INT32_MAX
template <typename Ret, typename T, typename... Args>
struct function_info<Ret(__thiscall*)(T*, Args...)> : member_function_info<call_type::thiscall_, Ret, T, Args...>
{
};
#endif
// template <typename Ret, typename T, typename... Args>
// struct non_member_function_info<call_type::thiscall_, Ret, T, Args...>
//     : member_function_info<call_type::thiscall_, Ret, T, Args...>
//{
// };

#define FN_INFO_MEMBER(call__, __call, _call_)                                                                            \
    template <typename Ret, typename T, typename... Args>                                                                 \
    struct function_info<Ret (__call T::*)(Args...)> : member_function_info<call__, Ret, T, Args...>                      \
    {                                                                                                                     \
    };                                                                                                                    \
    template <typename Ret, typename T, typename... Args>                                                                 \
    struct function_info<Ret (__call T::*)(Args...) noexcept> : member_function_info<call__, Ret, T, Args...>             \
    {                                                                                                                     \
    };                                                                                                                    \
    template <typename Ret, typename T, typename... Args>                                                                 \
    struct function_info<Ret (__call T::*)(Args...) const> : member_function_info<call__, Ret, T const, Args...>          \
    {                                                                                                                     \
    };                                                                                                                    \
    template <typename Ret, typename T, typename... Args>                                                                 \
    struct function_info<Ret (__call T::*)(Args...) const noexcept> : member_function_info<call__, Ret, T const, Args...> \
    {                                                                                                                     \
    };

X86_CALL_MEMBER(FN_INFO_MEMBER)
#undef FN_INFO_MEMBER

#define FN_INFO_NON_MEMEBER(call__, __call, _call_)                                                       \
    template <typename Ret, typename... Args>                                                             \
    struct function_info<Ret(__call*)(Args...)> : non_member_function_info<call__, Ret, Args...>          \
    {                                                                                                     \
    };                                                                                                    \
    template <typename Ret, typename... Args>                                                             \
    struct function_info<Ret(__call*)(Args...) noexcept> : non_member_function_info<call__, Ret, Args...> \
    {                                                                                                     \
    };
X86_CALL(FN_INFO_NON_MEMEBER)
#undef FN_INFO_NON_MEMEBER

namespace detail
{
template <class Call_T, typename Ret, class Object, typename... Args>
constexpr auto decay_function_info_helper(member_function_info<Call_T, Ret, Object, Args...>) -> member_function_info<Call_T, Ret, Object, Args...>
{
    return {};
}

template <class Call_T, typename Ret, typename... Args>
constexpr auto decay_function_info_helper(non_member_function_info<Call_T, Ret, Args...>) -> non_member_function_info<Call_T, Ret, Args...>
{
    return {};
}

struct call_type_sample
{
    void operator()() const
    {
    }

    static void fn()
    {
    }
};
} // namespace detail

template <class FnInfo>
using decay_function_info = decltype(detail::decay_function_info_helper(std::declval<FnInfo>()));

using default_call_type_member     = function_info<detail::call_type_sample>::call_type;
using default_call_type_non_member = function_info<decltype(&detail::call_type_sample::fn)>::call_type;
} // namespace fd
