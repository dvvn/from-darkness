#pragma once

#include "core.h"

#include <functional>

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

template <template <call_type_t Call> class Q, typename... Args>
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

struct member_func_gap
{
};

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

template <class Ret, typename Fn, class T, typename... Args>
struct member_func_invoker
{
    Ret operator()(Fn function, T *instance, Args... args) const
    {
        return std::invoke(function, instance, args...);
    }

    Ret operator()(void *function, T *instance, Args... args) const
    {
        return std::invoke(void_to_func<Fn>(function), instance, args...);
    }
};

template <class Ret, typename T, typename... Args>
struct non_member_func_invoker_thiscall // WIP, temp solution
{
    using type = Ret(__thiscall *)(T *, Args...);

    Ret operator()(type function, T *instance, Args... args) const
    {
        return std::invoke(function, instance, args...);
    }

    Ret operator()(void *function, T *instance, Args... args) const
    {
        return std::invoke(void_to_func<type>(function), instance, args...);
    }
};

// template <class Ret, typename T, typename... Args>
// struct member_func_invoker<Ret, Ret(__thiscall *)(T *, Args...), T, Args...>
//     : non_member_func_invoker_thiscall<Ret, T, Args...>
//{
// };

template <class Ret, typename Fn, typename... Args>
struct member_func_invoker<Ret, Fn, void, Args...>
{
    static_assert(std::invocable<Fn, member_func_gap *, Args...>);

    Ret operator()(Fn function, void *instance, Args... args) const
    {
        return std::invoke(function, static_cast<member_func_gap *>(instance), args...);
    }

    Ret operator()(void *function, void *instance, Args... args) const
    {
        return std::invoke(void_to_func<Fn>(function), static_cast<member_func_gap *>(instance), args...);
    }
};

template <class Ret, typename Fn, typename... Args>
struct member_func_invoker<Ret, Fn, member_func_gap, Args...> : member_func_invoker<Ret, Fn, void, Args...>
{
};

template <call_type_t Call, typename Ret, typename T, typename... Args>
requires(std::is_class_v<T>)
struct member_func_type;

#define MEMBER_FN_TYPE(call__, __call, _call_)            \
    template <typename Ret, typename T, typename... Args> \
    struct member_func_type<call__, Ret, T, Args...>      \
    {                                                     \
        using type = Ret (__call T::*)(Args...);          \
    };

X86_CALL_MEMBER(MEMBER_FN_TYPE)
#undef MEMBER_FN_BUILDER

template <call_type_t Call, typename Ret, typename T, typename... Args>
struct member_func_builder : member_func_type<Call, Ret, T, Args...>
{
    using type    = typename member_func_type<Call, Ret, T, Args...>::type;
    using invoker = member_func_invoker<Ret, type, T, Args...>;

    static constexpr auto call_type = Call;
    static constexpr auto get       = void_to_func<type>;
    static constexpr invoker invoke;
};

template <call_type_t Call, typename Ret, typename T, typename... Args>
using build_member_func = typename member_func_builder<Call, Ret, T, Args...>::type;

template <call_type_t Call, typename Ret, typename... Args>
struct member_func_builder<Call, Ret, void, Args...> : member_func_builder<Call, Ret, member_func_gap, Args...>
{
};

template <typename Ret, typename... Args>
struct member_func_builder<call_type_t::thiscall_, Ret, void, Args...>
{
    using invoker = non_member_func_invoker_thiscall<Ret, void, Args...>;

    static constexpr auto call_type = call_type_t::thiscall_;
    using type                      = typename invoker::type;
    static constexpr auto get       = void_to_func<type>;
    static constexpr invoker invoke;
};

template <typename Ret, typename T, typename... Args>
struct unknown_member_func_builder
{
    template <call_type_t Call>
#if 0    
    static auto get(void *function, call_type_holder<Call> = {})
    {
        return member_func_builder<Call, Ret, T, Args...>::get(function);
    }
#else
    static constexpr auto get = member_func_builder<Call, Ret, T, Args...>::get;
#endif

    template <call_type_t Call>
    static Ret invoke(void *function, T *instance, Args... args)
    {
        return member_func_builder<Call, Ret, T, Args...>::invoke(function, instance, args...);
    }

    template <call_type_t Call>
    struct invoke_proxy
    {
        Ret operator()(void *function, T *instance, Args... args) const
        {
            return invoke<Call>(function, instance, args...);
        }
    };

    static Ret invoke(void *function, T *instance, call_type_t info, Args... args)
    {
        return apply<invoke_proxy>(info, function, instance, args...);
    }

    template <member_function Fn>
    static Ret invoke(Fn function, T *instance, Args... args)
    {
        return invoke<get_call_type(function)>(get_function_pointer(function), instance, args...);
    }
};

template <call_type_t Call, typename T, typename... Args>
class member_func_return_type_resolver
{
    template <typename Ret>
    using builder = member_func_builder<Call, Ret, T, Args...>;

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
        return std::apply(std::bind_front(builder<Ret>::invoke, function_, instance_), args_);
    }
};
}