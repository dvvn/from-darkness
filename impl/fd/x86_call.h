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

template <_x86_call Call, typename Ret, typename... Args>
struct _x86_invoker;

template <_x86_call Call, typename Ret, typename... Args>
Ret x86_invoker(void *instance, void *fn, Args... args)
{
    return _x86_invoker<Call, Ret, Args...>::call(instance, fn, static_cast<Args>(args)...);
}

template <typename Ret, typename... Args>
Ret x86_invoker(void *instance, void *fn, _x86_call call, Args... args)
{
#define _X86_INVOKE(call__, __call) \
    case call__:                    \
        return x86_invoker<call__, Ret>(instance, fn, static_cast<Args>(args)...);

    switch (call)
    {
        X86_CALL_MEMBER(_X86_INVOKE);
    }

#undef _X86_INVOKE
}

#define X86_INVOKER(call__, __call)                                                    \
    template <typename Ret, typename... Args>                                          \
    struct _x86_invoker<call__, Ret, Args...>                                          \
    {                                                                                  \
        static Ret call(void *instance, void *fn, Args... args) noexcept               \
        {                                                                              \
            struct dummy                                                               \
            {                                                                          \
            };                                                                         \
            union                                                                      \
            {                                                                          \
                void *fn1;                                                             \
                Ret (__call dummy::*fn2)(Args...);                                     \
            };                                                                         \
            fn1 = fn;                                                                  \
            return (*static_cast<dummy *>(instance).*fn2)(static_cast<Args>(args)...); \
        }                                                                              \
    };

X86_CALL_MEMBER(X86_INVOKER);

#undef X86_INVOKER
}