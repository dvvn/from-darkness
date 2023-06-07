#pragma once

#include <utility>

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
    unknown,
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

template <typename Fn, call_type_t Call>
concept same_call_type = get_call_type(Fn()) == Call;

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

template <typename T>
concept member_function = requires(T fn) { get_call_type_member(fn); };

template <call_type_t Call, typename Ret, typename... Args>
struct member_func_invoker;

struct member_func_invoker_gap
{
};

#define MEMBER_FUNC_INVOKER(call__, __call, _call_)                                   \
    template <typename Ret, typename... Args>                                         \
    struct member_func_invoker<call__, Ret, Args...>                                  \
    {                                                                                 \
        static Ret __call call(void *instance, void *fn, Args... args) noexcept       \
        {                                                                             \
            union                                                                     \
            {                                                                         \
                void *fn1;                                                            \
                Ret (__call member_func_invoker_gap::*fn2)(Args...);                  \
            };                                                                        \
            fn1 = fn;                                                                 \
            return (*static_cast<member_func_invoker_gap *>(instance).*fn2)(args...); \
        }                                                                             \
    };

X86_CALL_MEMBER(MEMBER_FUNC_INVOKER);
#undef MEMBER_FUNC_INVOKER

template <typename Ret, typename... Args>
struct member_func_invoker<call_type_t::unknown, Ret, Args...>
{
    static Ret call(void *instance, void *fn, call_type_t info, Args... args) noexcept
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

    template <call_type_t Call>
    static Ret call(void *instance, void *fn, call_type_holder<Call>, Args... args) noexcept
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

template <call_type_t Call, typename Ret, typename T, typename... Args>
struct member_func_builder;

template <call_type_t Call, typename Ret, typename T, typename... Args>
using build_member_func = typename member_func_builder<Call, Ret, T, Args...>::type;

template <call_type_t Call, typename Ret, typename T, typename... Args>
struct member_func_caster
{
    using type = build_member_func<Call, Ret, T, Args>;

    static type get(void *function)
    {
        if constexpr (std::convertible_to<void *, type>)
            return static_cast<type>(function);
        else
        {
            static_assert(sizeof(type) == sizeof(void *));

            union
            {
                void *from;
                type to;
            };

            from = function;
            return to;
        }
    }
};

template <call_type_t Call, typename Ret, typename... Args>
struct member_func_builder<Call, Ret, void, Args...> : member_func_builder<Call, Ret, member_func_invoker_gap, Args...>
{
};

template <typename Ret, typename... Args>
struct member_func_builder<call_type_t::thiscall_, Ret, void, Args...>
    : member_func_caster<call_type_t::thiscall_, Ret, void, Args...>
{
    using type = Ret(__thiscall *)(void *, Args...);
};

#define MEMBER_FN_BUILDER(call__, __call, _call_)                                                     \
    template <typename Ret, typename T, typename... Args>                                             \
    struct member_func_builder<call__, Ret, T, Args...> : member_func_caster<call__, Ret, T, Args...> \
    {                                                                                                 \
        using type = Ret (__call T::*)(Args...);                                                      \
    };

X86_CALL_MEMBER(MEMBER_FN_BUILDER)
#undef MEMBER_FN_BUILDER
}