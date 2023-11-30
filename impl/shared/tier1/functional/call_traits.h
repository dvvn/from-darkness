#pragma once

// #define FD_SPOOF_RETURN_ADDRESS
#include "tier1/concepts.h"
#include "tier1/functional/cast.h"

#ifdef FD_SPOOF_RETURN_ADDRESS
#include <x86RetSpoof.h>
#endif

#include <functional>

namespace FD_TIER(1)
{
#define _CCV_TYPE(_CCV_, ...) \
    struct _CCV_##_ final     \
    {                         \
    };

struct call_type final
{
#ifdef _MSC_VER
    _MEMBER_CALL(_CCV_TYPE, , , );
#else

#endif
};

#undef _CCV_TYPE

#define _CCV_T(_CCV_) call_type::_CCV_##_

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

template <typename Fn>
struct function_info;

template <bool Noexcept, class Call_T, typename Ret, class T, typename... Args>
struct member_function_info;

template <bool Noexcept, class Call_T, typename Ret, typename... Args>
struct non_member_function_info;

template <bool Noexcept, class Call_T, typename Ret, typename Object, typename... Args>
// requires(std::is_class_v<Object> || std::is_union_v<Object> /*std::is_fundamental_v<T>*/)
struct member_function;

template <bool Noexcept, class Call_T, typename Ret, typename... Args>
struct non_member_function;

#define _NOEXCEPT_BOOL(...) 0##__VA_OPT__(1)

#define _MEMBER_CCV_HELPER(_CCV_, _CV_, _REF_, _NOEXCEPT_)                                               \
    template <typename Ret, typename Object, typename... Args>                                           \
    struct member_function<_NOEXCEPT_BOOL(_NOEXCEPT_), _CCV_T(_CCV_), Ret, Object _CV_ _REF_, Args...> : \
        std::type_identity<Ret (_CCV_ Object::*)(Args...) _CV_ _REF_ _NOEXCEPT_>                         \
    {                                                                                                    \
    };                                                                                                   \
    template <typename Ret, typename Object, typename... Args>                                           \
    struct function_info<Ret (_CCV_ Object::*)(Args...) _CV_ _REF_ _NOEXCEPT_> :                         \
        member_function_info<_NOEXCEPT_BOOL(_NOEXCEPT_), _CCV_T(_CCV_), Ret, Object _CV_ _REF_, Args...> \
    {                                                                                                    \
    };

#define _NON_MEMBER_CCV_HELPER(_CCV_, _CV_UNUSED_, _REF_UNUSED_, _NOEXCEPT_)                                                                  \
    template <typename Ret, typename... Args>                                                                                                 \
    struct non_member_function<_NOEXCEPT_BOOL(_NOEXCEPT_), _CCV_T(_CCV_), Ret, Args...> : std::type_identity<Ret(_CCV_*)(Args...) _NOEXCEPT_> \
    {                                                                                                                                         \
    };                                                                                                                                        \
    template <typename Ret, typename... Args>                                                                                                 \
    struct function_info<Ret(_CCV_*)(Args...) _NOEXCEPT_> : non_member_function_info<_NOEXCEPT_BOOL(_NOEXCEPT_), _CCV_T(_CCV_), Ret, Args...> \
    {                                                                                                                                         \
    };
#define _NON_MEMBER_THISCALL_HELPER(_CCV_, _CV_, _REF_, _NOEXCEPT_)                                      \
    template <typename Ret, typename Object, typename... Args>                                           \
    struct function_info<Ret(_CCV_*)(Object _CV_ * _REF_, Args...) _NOEXCEPT_> :                         \
        member_function_info<_NOEXCEPT_BOOL(_NOEXCEPT_), _CCV_T(_CCV_), Ret, Object _CV_ _REF_, Args...> \
    {                                                                                                    \
    };

#ifdef _MSC_VER
_MEMBER_CALL_CV_REF_NOEXCEPT(_MEMBER_CCV_HELPER)

_NON_MEMBER_CALL(_NON_MEMBER_CCV_HELPER, , , )
_NON_MEMBER_CALL(_NON_MEMBER_CCV_HELPER, , , noexcept)

#define _NON_MEMBER_CALL_THISCALL_CV(FUNC, REF_OPT, NOEXCEPT_OPT) \
    _EMIT_THISCALL(FUNC, , REF_OPT, NOEXCEPT_OPT)                 \
    _EMIT_THISCALL(FUNC, const, REF_OPT, NOEXCEPT_OPT)            \
    _EMIT_THISCALL(FUNC, volatile, REF_OPT, NOEXCEPT_OPT)         \
    _EMIT_THISCALL(FUNC, const volatile, REF_OPT, NOEXCEPT_OPT)

#define _NON_MEMBER_CALL_CV_THISCALL_REF(FUNC, NOEXCEPT_OPT) \
    _NON_MEMBER_CALL_THISCALL_CV(FUNC, , NOEXCEPT_OPT)       \
    _NON_MEMBER_CALL_THISCALL_CV(FUNC, &, NOEXCEPT_OPT)      \
    _NON_MEMBER_CALL_THISCALL_CV(FUNC, &&, NOEXCEPT_OPT)

#ifdef __cpp_noexcept_function_type
#define _NON_MEMBER_CALL_THISCALL_CV_REF_NOEXCEPT(FUNC) \
    _NON_MEMBER_CALL_CV_THISCALL_REF(FUNC, )            \
    _NON_MEMBER_CALL_CV_THISCALL_REF(FUNC, noexcept)
#else // __cpp_noexcept_function_type
#define _NON_MEMBER_CALL_THISCALL_CV_REF_NOEXCEPT(FUNC) _NON_MEMBER_CALL_CV_THISCALL_REF(FUNC, )
#endif // __cpp_noexcept_function_type

_NON_MEMBER_CALL_THISCALL_CV_REF_NOEXCEPT(_NON_MEMBER_THISCALL_HELPER)

#else

#endif

#undef _NON_MEMBER_CCV_HELPER
#undef _NON_MEMBER_THISCALL_HELPER
#undef _MEMBER_CCV_HELPER

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

template <bool Noexcept, class Call_T, typename Ret>
struct basic_function_info
{
    static constexpr bool no_throw = Noexcept;

    using call_type   = Call_T;
    using return_type = Ret;
};

template <bool Noexcept, class Call_T, typename Ret, class T, typename... Args>
struct member_function_info : basic_function_info<Noexcept, Call_T, Ret>
{
    template <template <bool, class, typename, class, typename...> class Other>
    using rebind = Other<Noexcept, Call_T, Ret, T, Args...>;

    using object_type_raw = T;
    using object_type     = std::remove_reference_t<T>;
    using args            = function_args<Args...>;
};

template <bool Noexcept, class Call_T, typename Ret, typename... Args>
struct non_member_function_info : basic_function_info<Noexcept, Call_T, Ret>
{
    template <template <bool, class, typename, typename...> class Other>
    using rebind = Other<Noexcept, Call_T, Ret, Args...>;

    using args = function_args<Args...>;
};

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

namespace detail
{
struct dummy_class final
{
};

template <bool Noexcept, class Call_T, typename Ret, class Object, typename... Args>
constexpr auto decay_function_info_helper(member_function_info<Noexcept, Call_T, Ret, Object, Args...> info) -> decltype(info)
{
    return {};
}

template <bool Noexcept, class Call_T, typename Ret, typename... Args>
constexpr auto decay_function_info_helper(non_member_function_info<Noexcept, Call_T, Ret, Args...> info) -> decltype(info)
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

template <typename Fn, typename... Args>
auto invoke(Fn&& fn, safe_cast_result<void, typename function_info<std::decay_t<Fn>>::object_type*> instance, Args&&... args)
{
    using object_type = typename function_info<std::decay_t<Fn>>::object_type;

#ifdef FD_SPOOF_RETURN_ADDRESS
    // WIP
#else
    return std::invoke(std::forward<Fn>(fn), safe_cast<object_type>(instance), std::forward<Args>(args)...);
#endif
}

template <typename Fn, typename Obj, typename... Args>
auto invoke(Fn&& fn, Obj* instance, Args&&... args) requires(
    sizeof(fn) == sizeof(void*) && !complete<typename function_info<std::decay_t<Fn>>::object_type>
#ifdef _DEBUG
    && std::invocable<Fn &&, Obj*, Args && ...>
#endif
)
{
#ifdef FD_SPOOF_RETURN_ADDRESS
    // WIP
#else
    return std::invoke(std::forward<Fn>(fn), unsafe_cast<detail::dummy_class*>(instance), std::forward<Args>(args)...);
#endif
}
} // namespace FD_TIER(1)
