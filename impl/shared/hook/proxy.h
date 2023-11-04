#pragma once

#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"
#include "hook/proxy_data.h"

namespace fd
{
#define FD_HOOK_PROXY_TEMPLATE template <class, typename, typename...>

namespace detail
{
template <class Call_T, typename Ret, class Object, typename... Args>
struct hook_proxy_member;

template <class Call_T, typename Ret, class Object, typename... Args>
class object_proxy_member
{
#if 0
    using proxy_type    = hook_proxy_member<Call_T, Ret, Object, Args...>;
    using proxy_pointer = std::conditional_t<std::is_const_v<Object>, proxy_type const, proxy_type>*;
#else
    using proxy_pointer = void const*;
#endif
    using object_pointer = Object*;

    using invoker_type = member_func_invoker<Call_T, Ret, Object, Args...>;

    void* original_;

    union
    {
        proxy_pointer proxy_;
        object_pointer object_;
    };

    [[no_unique_address]] //
    invoker_type invoker_;

  public:
    object_proxy_member(void* original, proxy_pointer proxy)
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

template <class Callback, class Proxy, typename... Args>
auto invoke_hook_proxy(Proxy* proxy, Args... args)
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
            return (invoke)(callback, original, proxy, args...);
        }
        else if constexpr (extra_args_count == 1)
        {
            using arg0 = std::remove_cvref_t<typename callback_args::template get<0>>;
            if constexpr (std::constructible_from<arg0, decltype(original), decltype(proxy)>)
                return (invoke)(callback, arg0(original, proxy), args...);
            else
                return (invoke)(callback, proxy, args...);
        }
    }
    else if constexpr (callback_args::count < args_count)
    {
#ifdef _DEBUG
        static_assert(callback_args::count <= 1);
#endif
        if constexpr (callback_args::count == 1)
            return (invoke)(callback, proxy);
        else if constexpr (callback_args::count == 0)
            return (invoke)(callback);
    }
    else
    {
        return (invoke)(callback, args...);
    }
}

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                              \
    template <typename Ret, class Object, typename... Args>                    \
    struct hook_proxy_member<call__, Ret, Object, Args...> : noncopyable       \
    {                                                                          \
        template <typename Callback>                                           \
        Ret __call proxy(Args... args) noexcept                                \
        {                                                                      \
            return invoke_hook_proxy<Callback>(this, args...);                 \
        }                                                                      \
    };                                                                         \
    template <typename Ret, class Object, typename... Args>                    \
    struct hook_proxy_member<call__, Ret, Object const, Args...> : noncopyable \
    {                                                                          \
        template <typename Callback>                                           \
        Ret __call proxy(Args... args) const noexcept                          \
        {                                                                      \
            return invoke_hook_proxy<Callback>(this, args...);                 \
        }                                                                      \
    };

X86_CALL_MEMBER(HOOK_PROXY_MEMBER);
#undef HOOK_PROXY_MEMBER

template <class Call_T, typename Ret, typename... Args>
struct hook_proxy_non_member;

template <class Call_T, typename Ret, typename... Args>
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

    using callback_args       = typename function_info<std::decay_t<decltype(callback)>>::args;
    constexpr auto args_count = sizeof...(Args);
    if constexpr (callback_args::count > args_count)
    {
        constexpr auto extra_args_count = callback_args::count - args_count;
        if constexpr (extra_args_count == 1)
        {
            using arg0 = std::remove_cvref_t<typename callback_args::template get<0>>;
            if constexpr (!std::is_fundamental_v<arg0> && std::constructible_from<arg0, decltype(original)>)
                return (invoke)(callback, arg0(original), args...);
            else
                return (invoke)(callback, original, args...);
        }
        else
            static_assert(always_false<Callback>);
    }
    else if constexpr (callback_args::count < args_count)
    {
        if constexpr (callback_args::count == 0)
            return (invoke)(callback, callback);
        else
            static_assert(always_false<Callback>);
    }
    else
    {
        return (invoke)(callback, args...);
    }
}

#define HOOK_PROXY_STATIC(call__, __call, _call_)                    \
    template <typename Ret, typename... Args>                        \
    struct hook_proxy_non_member<call__, Ret, Args...> : noncopyable \
    {                                                                \
        template <class Callback>                                    \
        static Ret __call proxy(Args... args) noexcept               \
        {                                                            \
            return invoke_hook_proxy<Callback, Args...>(args...);    \
        }                                                            \
    };

X86_CALL(HOOK_PROXY_STATIC);
#undef HOOK_PROXY_STATIC

template <class Proxy>
struct hook_proxy_extractor
{
    template <class Callback>
    static void* get()
    {
        return unsafe_cast<void*>(&Proxy::template proxy<Callback>);
    }
};

template <class Callback, class Proxy>
auto extract_hook_proxy()
{
    return hook_proxy_extractor<Proxy>::template get<Callback>();
}

template <class Callback, FD_HOOK_PROXY_TEMPLATE class Proxy, typename Func>
auto extract_hook_proxy(Func)
{
    using function_info = function_info<Func>;
    using proxy_type    = typename function_info::template rebind<Proxy>;
    return extract_hook_proxy<Callback, proxy_type>();
}

template <typename FnInfo>
struct default_hook_proxy;

template <class Call_T, typename Ret, class Object, typename... Args>
struct default_hook_proxy<member_function_info<Call_T, Ret, Object, Args...>> : hook_proxy_member<Call_T, Ret, Object, Args...>
{
};

template <class Call_T, typename Ret, typename... Args>
struct default_hook_proxy<non_member_function_info<Call_T, Ret, Args...>> : hook_proxy_non_member<Call_T, Ret, Args...>
{
};

template <typename Fn>
struct default_hook_proxy<function_info<Fn>> : default_hook_proxy<decay_function_info<function_info<Fn>>>
{
};
} // namespace detail

#if INTPTR_MAX == INT32_MAX || 0 // disabled forever, now automatically resolved
template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    typename Ret, class Object, typename... Args>
hook_info<Callback> prepare_hook(Ret(__thiscall* fn)(Object*, Args...))
{
    // using proxy_type = Proxy<call_type::thiscall_, Ret, Object, Args...>;
    return {unsafe_cast<void*>(fn), detail::extract_hook_proxy<Callback, Proxy>(fn)};
}
#endif

template <class Callback, FD_HOOK_PROXY_TEMPLATE class Proxy, typename Func>
hook_info<Callback> prepare_hook(Func fn)
{
    return {unsafe_cast<void*>(fn), detail::extract_hook_proxy<Callback, Proxy>(fn)};
}

template <class Callback, class Proxy, typename Func>
hook_info<Callback> prepare_hook(Func fn)
{
    return {unsafe_cast<void*>(fn), detail::extract_hook_proxy<Callback, Proxy>()};
}

template <class Callback, typename Func>
hook_info<Callback> prepare_hook(Func fn)
    // remove if no requires in vfunc prepare_hook
    requires(complete<function_info<Func>>)
{
    using fn_info = function_info<Func>;
    using proxy   = detail::default_hook_proxy<fn_info>;
    return prepare_hook<Callback, proxy>(fn);
}

#if 0 // old version
template <class Call_T, typename Ret, typename T, typename... Args>
struct vfunc;

template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    class Call_T, typename Ret, class Object, typename... Args>
hook_info<Callback> prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    using proxy = Proxy<Call_T, Ret, Object, Args...>;
    return prepare_hook<Callback, proxy>(target.get());
}

template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    class Call_T, typename Ret, class Object, typename... Args>
hook_info<Callback> prepare_hook(vfunc<Call_T, Ret, Object, function_args<Args...>> target)
{
    using proxy_type = Proxy<Call_T, Ret, Object, Args...>;
    return {target.get(), detail::extract_hook_proxy<Callback, proxy_type>()};
}
#else
struct vfunc_tag;

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, class VFunc>
hook_info<Callback> prepare_hook(VFunc target) requires
#ifdef _DEBUG
    requires { target.get_full(); } &&
#endif
    std::derived_from<VFunc, vfunc_tag>
{
    return prepare_hook<Callback, Proxy>(target.get_full());
}
#endif

#if 0 // probably unused
struct native_function_tag;

template <
    typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    std::derived_from<native_function_tag> Fn>
hook_info<Callback> prepare_hook(Fn abstract_fn)
{
    return prepare_hook<Callback, Proxy>(abstract_fn.get());
}
#endif

template <typename Fn, class Source /*= void*/>
struct object_froxy_for : object_froxy_for<decay_function_info<function_info<Fn>>, Source>
{
    using object_froxy_for<decay_function_info<function_info<Fn>>, Source>::object_froxy_for;
};

template <class Call_T, typename Ret, class Object, typename... Args, class Source>
struct object_froxy_for<member_function_info<Call_T, Ret, Object, Args...>, Source> : detail::object_proxy_member<Call_T, Ret, Object, Args...>
{
    using detail::object_proxy_member<Call_T, Ret, Object, Args...>::object_proxy_member;
};

template <class Call_T, typename Ret, typename... Args, class Source>
struct object_froxy_for<non_member_function_info<Call_T, Ret, Args...>, Source> : detail::object_proxy_non_member<Call_T, Ret, Args...>
{
    using detail::object_proxy_non_member<Call_T, Ret, Args...>::object_proxy_non_member;
};

} // namespace fd