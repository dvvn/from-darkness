#pragma once

#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"
#include "hook/proxy_data.h"

namespace fd
{
namespace detail
{
template <call_type Call_T, typename Ret, class C, typename... Args>
struct hook_proxy_member;

template <call_type Call_T, typename Ret, class Object, typename... Args>
class object_proxy_member
{
    using invoker_type = member_func_invoker<Call_T, Ret, Object, Args...>;

    void* original_;

    union
    {
        void* proxy_;
        Object* object_;
    };

    [[no_unique_address]] //
    invoker_type invoker_;

  public:
    object_proxy_member()
    {
        ignore_unused(this);
    }

    object_proxy_member(void* original, void* proxy)
        : original_(original)
        , proxy_(proxy)
    {
    }

    Ret operator()(Args... args) const
    {
        return invoker_(original_, object_, args...);
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

template <class Callback, call_type Call_T, typename Ret, class Object, typename... Args>
decltype(auto) invoke_hook_proxy(hook_proxy_member<Call_T, Ret, Object, Args...>* proxy, Args... args)
{
    decltype(auto) original = unique_hook_proxy_data<Callback>.get_original();
    decltype(auto) callback = unique_hook_proxy_data<Callback>.get_callback();

    using callback_args       = typename function_info<std::decay_t<decltype(callback)>>::args;
    constexpr auto args_count = sizeof...(Args);
    if constexpr (callback_args::count > args_count)
    {
        constexpr auto extra_args_count = callback_args::count - args_count;
#ifdef _DEBUG
        static_assert(extra_args_count != 0 && extra_args_count <= 2);
#endif
        if constexpr (extra_args_count == 2)
        {
            return callback(original, proxy, args...);
        }
        else if constexpr (extra_args_count == 1)
        {
            using arg0 = typename callback_args::template get<0>;
            if constexpr (std::constructible_from<arg0, decltype(original), decltype(proxy)>)
                return callback(arg0(original, proxy), args...);
            else
                return callback(proxy, args...);
        }
    }
    else if constexpr (callback_args::count < args_count)
    {
#ifdef _DEBUG
        static_assert(callback_args::count <= 1);
#endif
        if constexpr (callback_args::count == 1)
            return callback(proxy);
        else if constexpr (callback_args::count == 0)
            return callback();
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
    using invoker_type = non_member_func_invoker<Call_T, Ret, Args...>;

    void* original_;
    [[no_unique_address]] //
    invoker_type invoker_;

  public:
    object_proxy_non_member(void* original)
        : original_(original)
    {
    }

    Ret operator()(Args... args) const
    {
        return invoker_(original_, args...);
    }
};

template <class Callback, typename... Args>
decltype(auto) invoke_hook_proxy(Args... args)
{
    decltype(auto) original = unique_hook_proxy_data<Callback>.get_original();
    decltype(auto) callback = unique_hook_proxy_data<Callback>.get_callback();

    using callback_args = typename function_info<std::decay_t<decltype(callback)>>::args;

    constexpr auto args_count = sizeof...(Args);

    if constexpr (callback_args::count > args_count)
    {
        constexpr auto extra_args_count = callback_args::count - args_count;
        if constexpr (extra_args_count == 1)
        {
            using arg0 = typename callback_args::template get<0>;
            if constexpr (std::is_class_v<arg0> && std::constructible_from<arg0, decltype(original)>)
                return callback(arg0(original), args...);
            else
                return callback((original), args...);
        }
        else
            static_assert(always_false<Callback>);
    }
    else if constexpr (callback_args::count < args_count)
    {
        if constexpr (callback_args::count == 0)
            return callback();
        else
            static_assert(always_false<Callback>);
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
            return invoke_hook_proxy<Callback, Args...>(args...);          \
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
} // namespace detail

#define HOOK_PROXY_SAMPLE template <call_type, typename...>

#define GET_HOOK_PROXY_MEMBER(call__, __call, _call_)                                                          \
    template <                                                                                                 \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = detail::hook_proxy_member, /**/                        \
        typename Ret, class Object, typename... Args>                                                          \
    hook_info<Callback> prepare_hook(Ret (__call Object::*target)(Args...))                                    \
    {                                                                                                          \
        using proxy_type = Proxy<call__, Ret, Object, Args...>;                                                \
        return {unsafe_cast<void*>(target), detail::extract_hook_proxy<proxy_type>::template get<Callback>()}; \
    }

X86_CALL_MEMBER(GET_HOOK_PROXY_MEMBER)
#undef GET_HOOK_PROXY_MEMBER

#define GET_HOOK_PROXY_NON_MEMBER(call__, __call, _call_)                                                      \
    template <                                                                                                 \
        class Callback, HOOK_PROXY_SAMPLE class Proxy = detail::hook_proxy_non_member, /**/                    \
        typename Ret, typename... Args>                                                                        \
    hook_info<Callback> prepare_hook(Ret(__call* target)(Args...))                                             \
    {                                                                                                          \
        using proxy_type = Proxy<call__, Ret, Args...>;                                                        \
        return {unsafe_cast<void*>(target), detail::extract_hook_proxy<proxy_type>::template get<Callback>()}; \
    }

X86_CALL(GET_HOOK_PROXY_NON_MEMBER)
#undef GET_HOOK_PROXY_NON_MEMBER

#if INTPTR_MAX == INT32_MAX
template <
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = detail::hook_proxy_member, //
    typename Ret, class Object, typename... Args>
hook_info prepare_hook(Ret(__thiscall* target)(Object*, Args...))
{
    using proxy_type = Proxy<call_type::thiscall_, Ret, Object, Args...>;
    return {unsafe_cast<void*>(target), detail::extract_hook_proxy<proxy_type>::template get<Callback>()};
}
#endif

#if 0 // unused
template <typename Callback>
concept hook_callback_know_function = requires { typename Callback::function_type; };

template <hook_callback_know_function Callback, HOOK_PROXY_SAMPLE class Proxy>
hook_info<Callback> prepare_hook(void* target)
{
    using function_type = typename Callback::function_type;
    return prepare_hook<Callback, Proxy>(unsafe_cast<function_type>(target));
}

template <hook_callback_know_function Callback>
hook_info<Callback> prepare_hook(void* target)
{
    using function_type = typename Callback::function_type;
    return prepare_hook<Callback>(unsafe_cast<function_type>(target));
}
#endif

template <
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = detail::hook_proxy_non_member, //
    call_type Call_T, typename Ret, typename... Args>
hook_info<Callback> prepare_hook(void* target)
{
    using proxy_type = Proxy<Call_T, Ret, Args...>;
    return {target, detail::extract_hook_proxy<proxy_type>::template get<Callback>()};
}

template <call_type Call_T, typename Ret, typename T, typename... Args>
struct vfunc;

template <
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = detail::hook_proxy_member, //
    call_type Call_T, typename Ret, class Object, typename... Args>
hook_info<Callback> prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    using proxy_type = Proxy<Call_T, Ret, Object, Args...>;
    return {target.get(), detail::extract_hook_proxy<proxy_type>::template get<Callback>()};
}

#if 0
struct native_function_tag;

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    std::derived_from<native_function_tag> Fn>
hook_info prepare_hook(Fn abstract_fn)
{
    return prepare_hook<Callback, Proxy>(abstract_fn.get());
}
#endif

#undef HOOK_PROXY_SAMPLE

template <typename Fn>
struct object_froxy_for;

template <call_type Call_T, typename Ret, class Object, typename... Args>
struct object_froxy_for<member_function_info<Call_T, Ret, Object, Args...>> : std::type_identity<detail::object_proxy_member<Call_T, Ret, Object, Args...>>
{
};
#if 1
template <call_type Call_T, typename Ret, typename... Args>
struct object_froxy_for<non_member_function_info<Call_T, Ret, Args...>> : std::type_identity<detail::object_proxy_non_member<Call_T, Ret, Args...>>
{
};
#else
template <call_type Call_T, typename Ret, typename... Args>
struct object_froxy_for<non_member_function_info<Call_T, Ret, Args...>> : non_member_function<Call_T, Ret, Args...>
{
};
#endif

template <typename Fn>
struct object_froxy_for<function_info<Fn>> : object_froxy_for<typename function_info<Fn>::base>
{
};
} // namespace fd