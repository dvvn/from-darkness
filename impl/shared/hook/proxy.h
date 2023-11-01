#pragma once

#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"
#include "hook/proxy_data.h"

namespace fd
{
#define FD_HOOK_PROXY_TEMPLATE template <call_type, typename...>

namespace detail
{
template <call_type Call_T, typename Ret, class Object, typename... Args>
struct hook_proxy_member;

template <call_type Call_T, typename Ret, class Object, typename... Args>
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
decltype(auto) invoke_hook_proxy(Proxy* proxy, Args... args)
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
            using arg0 = std::remove_cvref_t<typename callback_args::template get<0>>;
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

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                                    \
    template <typename Ret, class Object, typename... Args>                          \
    struct hook_proxy_member<call__, Ret, Object, Args...> final : noncopyable       \
    {                                                                                \
        template <typename Callback>                                                 \
        Ret __call proxy(Args... args) noexcept                                      \
        {                                                                            \
            return invoke_hook_proxy<Callback>(this, args...);                       \
        }                                                                            \
    };                                                                               \
    template <typename Ret, class Object, typename... Args>                          \
    struct hook_proxy_member<call__, Ret, Object const, Args...> final : noncopyable \
    {                                                                                \
        template <typename Callback>                                                 \
        Ret __call proxy(Args... args) const noexcept                                \
        {                                                                            \
            return invoke_hook_proxy<Callback>(this, args...);                       \
        }                                                                            \
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
            using arg0 = std::remove_cvref_t<typename callback_args::template get<0>>;
            if constexpr (!std::is_fundamental_v<arg0> && std::constructible_from<arg0, decltype(original)>)
                return callback(arg0(original), args...);
            else
                return callback(original, args...);
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

#define HOOK_PROXY_STATIC(call__, __call, _call_)                          \
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

template <typename Fn>
struct default_hook_proxy;

template <call_type Call_T, typename Ret, class Object, typename... Args>
struct default_hook_proxy<member_function_info<Call_T, Ret, Object, Args...>> : std::type_identity<hook_proxy_member<Call_T, Ret, Object, Args...>>
{
};

template <call_type Call_T, typename Ret, typename... Args>
struct default_hook_proxy<non_member_function_info<Call_T, Ret, Args...>> : std::type_identity<hook_proxy_non_member<Call_T, Ret, Args...>>
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
    using fn_info = typename function_info<Func>::base;
    using proxy   = typename detail::default_hook_proxy<fn_info>::type;
    return prepare_hook<Callback, proxy>(fn);
}

template <call_type Call_T, typename Ret, typename T, typename... Args>
struct vfunc;

#if 0 // old version
template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    call_type Call_T, typename Ret, class Object, typename... Args>
hook_info<Callback> prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    using proxy = Proxy<Call_T, Ret, Object, Args...>;
    return prepare_hook<Callback, proxy>(target.get());
}

template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy_member, //
    call_type Call_T, typename Ret, class Object, typename... Args>
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

template <typename Fn>
struct object_froxy_for;

template <call_type Call_T, typename Ret, class Object, typename... Args>
struct object_froxy_for<member_function_info<Call_T, Ret, Object, Args...>> : std::type_identity<detail::object_proxy_member<Call_T, Ret, Object, Args...>>
{
};

template <call_type Call_T, typename Ret, typename... Args>
struct object_froxy_for<non_member_function_info<Call_T, Ret, Args...>> : std::type_identity<detail::object_proxy_non_member<Call_T, Ret, Args...>>
{
};
} // namespace fd