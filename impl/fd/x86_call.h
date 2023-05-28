#pragma once

#include <utility>

namespace fd
{
#undef cdecl
// ReSharper disable CppInconsistentNaming
enum class _x86_call : uint8_t
{
    thiscall__,
    cdecl__,
    fastcall__,
    stdcall__,
    vectorcall__,
    unknown,
};

#if 0
#define X86_CALL_PROXY(call__, __call, call)
#define X86_CALL_PROXY_MEMBER(call__, __call, call)
#endif

#define _X86_CALL_PROXY(_PROXY_, _T_) _PROXY_(_x86_call::_T_##__, __##_T_, ##_T_)

#define X86_CALL(_PROXY_)              \
    _X86_CALL_PROXY(_PROXY_, cdecl)    \
    _X86_CALL_PROXY(_PROXY_, fastcall) \
    _X86_CALL_PROXY(_PROXY_, stdcall)  \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER(_PROXY_)       \
    _X86_CALL_PROXY(_PROXY_, thiscall) \
    X86_CALL(_PROXY_)

#define X86_CALL_TYPE(call__, __call, call)                                    \
    template <typename Ret, typename T, typename... Args>                      \
    constexpr _x86_call get_call_type(Ret (__call T::*)(Args...))              \
    {                                                                          \
        return call__;                                                         \
    }                                                                          \
    template <typename Ret, typename T, typename... Args>                      \
    constexpr _x86_call get_call_type_member(Ret (__call T::*)(Args...))       \
    {                                                                          \
        return call__;                                                         \
    }                                                                          \
    template <typename Ret, typename T, typename... Args>                      \
    constexpr _x86_call get_call_type(Ret (__call T::*)(Args...) const)        \
    {                                                                          \
        return call__;                                                         \
    }                                                                          \
    template <typename Ret, typename T, typename... Args>                      \
    constexpr _x86_call get_call_type_member(Ret (__call T::*)(Args...) const) \
    {                                                                          \
        return call__;                                                         \
    }                                                                          \
    template <typename Ret, typename... Args>                                  \
    constexpr _x86_call get_call_type(Ret(__call *)(Args...))                  \
    {                                                                          \
        return call__;                                                         \
    }

X86_CALL_MEMBER(X86_CALL_TYPE);
#undef X86_CALL_TYPE

template <typename Fn, _x86_call Call>
concept same_call_type = get_call_type(Fn()) == Call;

template <_x86_call C>
struct call_type_t
{
    static constexpr _x86_call value = C;

    constexpr operator _x86_call() const
    {
        return C;
    }
};

namespace call_type
{
#define X86_CALL_TYPE(call__, __call, call) constexpr call_type_t<call__> call##_;
X86_CALL_MEMBER(X86_CALL_TYPE)
#undef X86_CALL_TYPE
} // namespace call_type

template <typename T>
concept member_function = requires(T fn) { get_call_type_member(fn); };

template <_x86_call Call, typename Ret, typename... Args>
struct member_func_invoker;

struct member_func_invoker_gap
{
};

#define MEMBER_FUNC_INVOKER(call__, __call, _call_)                                     \
    template <typename Ret, typename... Args>                                           \
    struct member_func_invoker<call__, Ret, Args...>                                    \
    {                                                                                   \
        static Ret __call call(void *instance, void *fn, Args... args) noexcept         \
        {                                                                               \
            union                                                                       \
            {                                                                           \
                void *fn1;                                                              \
                Ret (__call member_func_invoker_gap::*fn2)(Args...);                    \
            };                                                                          \
            fn1 = fn;                                                                   \
            return (*static_cast<member_func_invoker_gap *>(instance).*fn2)((args)...); \
        }                                                                               \
    };

X86_CALL_MEMBER(MEMBER_FUNC_INVOKER);
#undef MEMBER_FUNC_INVOKER

template <typename Ret, typename... Args>
struct member_func_invoker<_x86_call::unknown, Ret, Args...>
{
    static Ret call(void *instance, void *fn, _x86_call info, Args... args) noexcept
    {
#define SELECT_INVOKER(call__, __call, _call_) \
    case call__:                               \
        return member_func_invoker<call__, Ret, Args...>::call(instance, fn, (args)...);

        switch (info)
        {
            X86_CALL_MEMBER(SELECT_INVOKER);
        default:
            std::unreachable();
        }

#undef SELECT_INVOKER
    }

    template <_x86_call Call>
    static Ret call(void *instance, void *fn, call_type_t<Call>, Args... args) noexcept
    {
        return member_func_invoker<Call, Ret, Args...>::call(instance, fn, (args)...);
    }

    template <member_function Fn>
    static Ret call(void *instance, Fn fn, Args... args) noexcept
    {
        constexpr auto call_t = get_call_type(fn);
        auto func_ptr         = get_function_pointer(fn);

        return member_func_invoker<call_t, Ret, Args...>::call(instance, func_ptr, (args)...);
    }
};
}