﻿#pragma once

#include "basic_backend.h"
#include "functional/call_traits.h"

#include <source_location>

namespace fd
{
namespace detail
{
template <typename T>
class non_default_constructible_hook_callback final : public noncopyable
{
    uint8_t buff_[sizeof(T)];

  public:
    non_default_constructible_hook_callback()
    {
        (void)buff_;
    }

    non_default_constructible_hook_callback(T &obj)
    {
        reinterpret_cast<T &&>(buff_) = obj;
    }

    non_default_constructible_hook_callback(T &&obj)
    {
        reinterpret_cast<T &&>(buff_) = static_cast<T &&>(obj);
    }

    operator T &()
    {
        return reinterpret_cast<T &>(buff_);
    }

    T *operator&()
    {
        return reinterpret_cast<T *>(&buff_);
    }
};

template <typename Callback, size_t N>
struct hook_callback_reference final : noncopyable
{
};

template <typename Callback>
struct hook_callback
{
    using type = Callback;
};

template <typename Callback, size_t N>
struct hook_callback<detail::hook_callback_reference<Callback, N>> : hook_callback<Callback>
{
};

constexpr size_t hook_reference_index(std::source_location const &loc)
{
    return loc.column() + loc.line();
}

} // namespace detail

template <typename Callback, bool = std::is_default_constructible_v<Callback>>
inline Callback unique_hook_callback;

template <typename Callback>
inline detail::non_default_constructible_hook_callback<Callback> unique_hook_callback<Callback, false>;

template <typename Callback>
inline void *unique_hook_trampoline;

template <typename Callback, size_t N, bool V>
inline Callback &unique_hook_callback<detail::hook_callback_reference<Callback, N>, V> = unique_hook_callback<Callback>;

template <typename Callback>
using hook_callback_t = typename detail::hook_callback<Callback>::type;

template <typename Callback>
void init_hook_callback(Callback &callback)
{
    static_assert(std::is_trivially_destructible_v<Callback>);
    unique_hook_callback<Callback> = static_cast<Callback &&>(callback);
}

template <typename Callback, typename... Args>
void init_hook_callback(Args &&...args)
{
    static_assert(std::is_trivially_destructible_v<Callback>);
    new (&unique_hook_callback<Callback>) Callback(std::forward<Args>(args)...);
}

template <call_type Call_T, typename Ret, class C, typename... Args>
struct hook_proxy_member;

template <call_type Call_T, typename Ret, class Object, typename... Args>
class object_proxy_member
{
    void *original_;

    union
    {
        void *proxy_;
        Object *object_;
    };

  public:
    object_proxy_member(void *original, void *proxy)
        : original_(original)
        , proxy_(proxy)
    {
    }

    Ret operator()(Args... args)
    {
        member_func_invoker<Call_T, Ret, Object, Args...> invoker;
        return invoker(original_, object_, args...);
    }

    operator Object *()
    {
        return object_;
    }

    Object *operator->()
    {
        return object_;
    }
};

#define HOOK_PROXY_SAMPLE template <call_type, typename...>

template <class Callback, call_type Call_T, typename Ret, class Object, typename... Args>
Ret invoke_hook_proxy(hook_proxy_member<Call_T, Ret, Object, Args...> *proxy, Args... args)
{
    using original_proxy = object_proxy_member<Call_T, Ret, Object, Args...>;
    using callback_type  = hook_callback_t<Callback>;

    callback_type &callback = unique_hook_callback<Callback>;

    constexpr auto pass_original = std::invocable<callback_type, original_proxy &, Args...>;
    constexpr auto pass_classptr = std::invocable<callback_type, Object *, Args...>;

    if constexpr (pass_original)
    {
        original_proxy obj(unique_hook_trampoline<Callback>, proxy);
        return callback(obj, args...);
    }
    else if constexpr (pass_classptr)
    {
        auto classptr = unsafe_cast<Object *>(proxy);
        return callback(classptr, args...);
    }
    else
    {
        return callback(args...);
    }
}

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                         \
    template <typename Ret, class C, typename... Args>                    \
    struct hook_proxy_member<call__, Ret, C, Args...> final : noncopyable \
    {                                                                     \
        template <typename Callback>                                      \
        Ret __call proxy(Args... args) noexcept                           \
        {                                                                 \
            return invoke_hook_proxy<Callback>(this, args...);            \
        }                                                                 \
    };

X86_CALL_MEMBER(HOOK_PROXY_MEMBER);
#undef HOOK_PROXY_MEMBER

template <call_type Call_T, typename Ret, typename... Args>
struct hook_proxy_non_member;

template <call_type Call_T, typename Ret, typename... Args>
class object_proxy_non_member
{
    void *original_;

  public:
    object_proxy_non_member(void *original)
        : original_(original)
    {
    }

    Ret operator()(Args... args)
    {
        non_member_func_invoker<Call_T, Ret, Args...> invoker;
        return invoker(original_, args...);
    }
};

template <class Callback, call_type Call_T, typename Ret, typename... Args>
Ret invoke_hook_proxy(Args... args)
{
    using original       = non_member_func_type<Call_T, Ret, Args...>;
    using original_proxy = object_proxy_non_member<Call_T, Ret, Args...>; // or std::bind
    using callback_type  = hook_callback_t<Callback>;

    callback_type &callback = unique_hook_callback<Callback>;

    constexpr auto pass_original       = std::invocable<callback_type, original, Args...>;
    constexpr auto pass_original_proxy = std::invocable<callback_type, original_proxy &, Args...>;

    if constexpr (pass_original)
    {
        auto fn = unsafe_cast<original>(unique_hook_trampoline<Callback>);
        return callback(fn, args...);
    }
    else if constexpr (pass_original_proxy)
    {
        original_proxy holder(unique_hook_trampoline<Callback>);
        return callback(holder, args...);
    }
    else
    {
        return callback(args...);
    }
}

#define HOOK_PROXY_STATIC(call__, __call, call)                            \
    template <typename Ret, typename... Args>                              \
    struct hook_proxy_non_member<call__, Ret, Args...> final : noncopyable \
    {                                                                      \
        template <class Callback>                                          \
        static Ret __call proxy(Args... args) noexcept                     \
        {                                                                  \
            return invoke_hook_proxy<Callback, call__, Ret>(args...);      \
        }                                                                  \
    };

X86_CALL(HOOK_PROXY_STATIC);
#undef HOOK_PROXY_STATIC

template <class Proxy>
struct extract_hook_proxy;

template <call_type Call_T, typename Ret, typename Object, typename... Args>
struct extract_hook_proxy<hook_proxy_member<Call_T, Ret, Object, Args...>>
{
    template <class Callback>
    static void *get()
    {
        return unsafe_cast<void *>(&hook_proxy_member<Call_T, Ret, Object, Args...>::template proxy<Callback>);
    }
};

template <call_type Call_T, typename Ret, typename... Args>
struct extract_hook_proxy<hook_proxy_non_member<Call_T, Ret, Args...>>
{
    template <class Callback>
    static void *get()
    {
        return unsafe_cast<void *>(&hook_proxy_non_member<Call_T, Ret, Args...>::template proxy<Callback>);
    }
};

#define GET_HOOK_PROXY_MEMBER(call__, __call, _call_)                                          \
    template <                                                                                 \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, /**/                \
        typename Ret, class Object, typename... Args>                                          \
    prepared_hook_data prepare_hook(Ret (__call Object::*target)(Args...))                     \
    {                                                                                          \
        return {                                                                               \
            unsafe_cast<void *>(target), /**/                                                  \
            extract_hook_proxy<Proxy<call__, Ret, Object, Args...>>::template get<Callback>(), \
            &unique_hook_trampoline<Callback> /**/                                             \
        };                                                                                     \
    }

X86_CALL_MEMBER(GET_HOOK_PROXY_MEMBER)
#undef GET_HOOK_PROXY_MEMBER

#define GET_HOOK_PROXY_NON_MEMBER(call__, __call, _call_)                              \
    template <                                                                         \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_non_member, /**/    \
        typename Ret, typename... Args>                                                \
    prepared_hook_data prepare_hook(Ret(__call *target)(Args...))                      \
    {                                                                                  \
        return {                                                                       \
            unsafe_cast<void *>(target), /**/                                          \
            extract_hook_proxy<Proxy<call__, Ret, Args...>>::template get<Callback>(), \
            &unique_hook_trampoline<Callback> /**/                                     \
        };                                                                             \
    }

X86_CALL(GET_HOOK_PROXY_NON_MEMBER)
#undef GET_HOOK_PROXY_NON_MEMBER

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    typename Ret, class Object, typename... Args>
prepared_hook_data prepare_hook(Ret(__thiscall *target)(Object *, Args...))
{
    return {
        unsafe_cast<void *>(target), //
        extract_hook_proxy<Proxy<call_type::thiscall_, Ret, Object, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}

template <typename Callback, HOOK_PROXY_SAMPLE class Proxy>
prepared_hook_data prepare_hook(void *target)
{
    return prepare_hook<Callback, Proxy>(unsafe_cast<typename hook_callback_t<Callback>::function_type>(target));
}

template <typename Callback>
prepared_hook_data prepare_hook(void *target)
{
    return prepare_hook<Callback>(unsafe_cast<typename hook_callback_t<Callback>::function_type>(target));
}

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_non_member, //
    call_type Call_T, typename Ret, typename... Args>
prepared_hook_data prepare_hook(void *target)
{
    return {
        (target), //
        extract_hook_proxy<Proxy<Call_T, Ret, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}

template <call_type Call_T, typename Ret, typename T, typename... Args>
class vfunc;

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    call_type Call_T, typename Ret, class Object, typename... Args>
prepared_hook_data prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    return {
        target.get(), //
        extract_hook_proxy<Proxy<Call_T, Ret, Object, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}

struct native_function_tag;

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    std::derived_from<native_function_tag> Fn>
prepared_hook_data prepare_hook(Fn abstract_fn)
{
    return prepare_hook<Callback, Proxy>(abstract_fn.get());
}

//----

template <typename Callback>
using hook_callback_ref = detail::hook_callback_reference<
    Callback,
#ifdef __RESHARPER__
    0
#else
    detail::hook_reference_index(std::source_location::current())
#endif
    >;

template <typename Callback>
prepared_hook_data prepare_hook_ex(auto target, Callback callback)
{
    init_hook_callback(std::move(callback));
    return prepare_hook<Callback>(target);
}

template <HOOK_PROXY_SAMPLE class Proxy, typename Callback>
prepared_hook_data prepare_hook_ex(auto target, Callback callback)
{
    init_hook_callback(std::move(callback));
    return prepare_hook<Callback, Proxy>(target);
}

template <typename Callback, typename... Args>
prepared_hook_data prepare_hook_ex(auto target, Args &&...args)
{
    init_hook_callback<Callback>(std::forward<Args>(args)...);
    return prepare_hook<Callback>(target);
}

template <HOOK_PROXY_SAMPLE class Proxy, typename Callback, typename... Args>
prepared_hook_data prepare_hook_ex(auto target, Args &&...args)
{
    init_hook_callback<Callback>(std::forward<Args>(args)...);
    return prepare_hook<Callback, Proxy>(target);
}

#undef HOOK_PROXY_SAMPLE
} // namespace fd