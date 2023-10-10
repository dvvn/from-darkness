#pragma once

#include "callback.h"
#include "prepared_data.h"
#include "functional/call_traits.h"

namespace fd
{
template <typename Callback>
inline Callback* unique_hook_callback;

template <typename Callback>
inline void* unique_hook_trampoline;

// template <typename Callback, typename... Args>
// void init_hook_callback(Args &&...args)
//{
//     static_assert(std::is_trivially_destructible_v<Callback>);
//     new (&unique_hook_callback<Callback>) Callback(std::forward<Args>(args)...);
// }

template <call_type Call_T, typename Ret, class C, typename... Args>
struct hook_proxy_member;

template <call_type Call_T, typename Ret, class Object, typename... Args>
class object_proxy_member
{
    void* original_;

    union
    {
        void* proxy_;
        Object* object_;
    };

  public:
    object_proxy_member(void* original, void* proxy)
        : original_(original)
        , proxy_(proxy)
    {
    }

    Ret operator()(Args... args)
    {
        member_func_invoker<Call_T, Ret, Object, Args...> invoker;
        return invoker(original_, object_, args...);
    }

    operator Object*()
    {
        return object_;
    }

    Object* operator->()
    {
        return object_;
    }
};

#define HOOK_PROXY_SAMPLE template <call_type, typename...>

template <class Callback>
decltype(auto) get_hook_callback()
{
    if constexpr (!hook_callback_need_protect<Callback>)
        return *unique_hook_callback<Callback>;
    else
        return []<typename... Args>(Args&&... args) {
            auto* callback = unique_hook_callback<Callback>;
            hook_callback_thread_protector const protector(callback);
            return (*callback)(std::forward<Args>(args)...);
        };
}

template <class Callback, call_type Call_T, typename Ret, class Object, typename... Args>
Ret invoke_hook_proxy(hook_proxy_member<Call_T, Ret, Object, Args...>* proxy, Args... args)
{
    using original_proxy = object_proxy_member<Call_T, Ret, Object, Args...>;

    decltype(auto) callback = get_hook_callback<Callback>();

    constexpr auto pass_original = std::invocable<Callback, original_proxy&, Args...>;
    constexpr auto pass_classptr = std::invocable<Callback, Object*, Args...>;

    if constexpr (pass_original)
    {
        original_proxy obj(unique_hook_trampoline<Callback>, proxy);
        return callback(obj, args...);
    }
    else if constexpr (pass_classptr)
    {
        auto classptr = unsafe_cast<Object*>(proxy);
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
    void* original_;

  public:
    object_proxy_non_member(void* original)
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

    decltype(auto) callback = get_hook_callback<Callback>();

    constexpr auto pass_original       = std::invocable<Callback, original, Args...>;
    constexpr auto pass_original_proxy = std::invocable<Callback, original_proxy&, Args...>;

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
    static void* get()
    {
        return unsafe_cast<void*>(&hook_proxy_member<Call_T, Ret, Object, Args...>::template proxy<Callback>);
    }
};

template <call_type Call_T, typename Ret, typename... Args>
struct extract_hook_proxy<hook_proxy_non_member<Call_T, Ret, Args...>>
{
    template <class Callback>
    static void* get()
    {
        return unsafe_cast<void*>(&hook_proxy_non_member<Call_T, Ret, Args...>::template proxy<Callback>);
    }
};

#define GET_HOOK_PROXY_MEMBER(call__, __call, _call_)                                                                                 \
    template <                                                                                                                        \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, /**/                                                       \
        typename Ret, class Object, typename... Args>                                                                                 \
    prepared_hook_data prepare_hook(Ret (__call Object::*target)(Args...))                                                            \
    {                                                                                                                                 \
        return {                                                                                                                      \
            unsafe_cast<void*>(target),                                                                                          /**/ \
            extract_hook_proxy<Proxy<call__, Ret, Object, Args...>>::template get<Callback>(), &unique_hook_trampoline<Callback> /**/ \
        };                                                                                                                            \
    }

X86_CALL_MEMBER(GET_HOOK_PROXY_MEMBER)
#undef GET_HOOK_PROXY_MEMBER

#define GET_HOOK_PROXY_NON_MEMBER(call__, __call, _call_)                                                                     \
    template <                                                                                                                \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_non_member, /**/                                           \
        typename Ret, typename... Args>                                                                                       \
    prepared_hook_data prepare_hook(Ret(__call* target)(Args...))                                                             \
    {                                                                                                                         \
        return {                                                                                                              \
            unsafe_cast<void*>(target),                                                                                  /**/ \
            extract_hook_proxy<Proxy<call__, Ret, Args...>>::template get<Callback>(), &unique_hook_trampoline<Callback> /**/ \
        };                                                                                                                    \
    }

X86_CALL(GET_HOOK_PROXY_NON_MEMBER)
#undef GET_HOOK_PROXY_NON_MEMBER

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    typename Ret, class Object, typename... Args>
prepared_hook_data prepare_hook(Ret(__thiscall* target)(Object*, Args...))
{
    return {
        unsafe_cast<void*>(target), //
        extract_hook_proxy<Proxy<call_type::thiscall_, Ret, Object, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}

template <typename Callback>
concept hook_callback_know_function = requires { typename Callback::function_type; };

template <hook_callback_know_function Callback, HOOK_PROXY_SAMPLE class Proxy>
prepared_hook_data prepare_hook(void* target)
{
    using function_type = typename Callback::function_type;
    return prepare_hook<Callback, Proxy>(unsafe_cast<function_type>(target));
}

template <hook_callback_know_function Callback>
prepared_hook_data prepare_hook(void* target)
{
    using function_type = typename Callback::function_type;
    return prepare_hook<Callback>(unsafe_cast<function_type>(target));
}

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_non_member, //
    call_type Call_T, typename Ret, typename... Args>
prepared_hook_data prepare_hook(void* target)
{
    return {
        target, //
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

#undef HOOK_PROXY_SAMPLE
} // namespace fd