#pragma once

#include <cstdint>

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
#define X86_CALL_PROXY(call__, __call)
#define X86_CALL_PROXY_MEMBER(call__, __call)
#endif

#define _X86_CALL_PROXY(_PROXY_, _T_) _PROXY_(_x86_call::_T_##__, __##_T_)

#define X86_CALL(_PROXY_)              \
    _X86_CALL_PROXY(_PROXY_, cdecl)    \
    _X86_CALL_PROXY(_PROXY_, fastcall) \
    _X86_CALL_PROXY(_PROXY_, stdcall)  \
    _X86_CALL_PROXY(_PROXY_, vectorcall)
#define X86_CALL_MEMBER(_PROXY_)       \
    _X86_CALL_PROXY(_PROXY_, thiscall) \
    X86_CALL(_PROXY_)

#define X86_CALL_TYPE(call__, __call)                                          \
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

template <typename T>
concept member_function = requires(T fn) { get_call_type_member(fn); };

class call_type
{
    _x86_call type_;

  public:
    template <typename Fn>
    consteval call_type(Fn function)
        : type_(get_call_type(function))
    {
    }

    constexpr operator _x86_call() const
    {
        return type_;
    }
};
}