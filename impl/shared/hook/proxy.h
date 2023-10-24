#pragma once

#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/prepared_data.h"

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

namespace detail
{
template <bool V, typename T, typename Nothing = std::false_type>
using type_or_nothing = std::conditional_t<V, T, std::false_type>;
}

template <class Callback, call_type Call_T, typename Ret, class Object, typename... Args>
class hook_callback_invoker_member
{
    using object_type         = hook_proxy_member<Call_T, Ret, Object, Args...>;
    using original_fn         = member_func_type<Call_T, Ret, Object, Args...>;
    using original_fn_wrapped = object_proxy_member<Call_T, Ret, Object, Args...>;

    template <typename... ExtraArgs>
    static constexpr bool invocable = std::invocable<Callback, ExtraArgs..., Args...>;

    static constexpr bool call_original_thisptr = invocable<original_fn, object_type*>;
    static constexpr bool call_thisptr_original = invocable<object_type*, original_fn>;

    static constexpr bool call_object_ptr = invocable<object_type*>;
    static constexpr bool call_original   = invocable<original_fn>;
    template <bool Const>
    static constexpr bool call_original_wrapped_ex = invocable<std::conditional_t<Const, original_fn_wrapped const&, original_fn_wrapped>>;
    static constexpr bool call_original_wrapped    = call_original_wrapped_ex<true> || call_original_wrapped_ex<false>;

#ifdef _DEBUG
    static_assert(call_original_thisptr + call_thisptr_original + call_object_ptr + call_original + call_original_wrapped == 1);
#endif

    static constexpr bool store_original_and_thisptr = call_original_thisptr + call_thisptr_original;

    Callback* callback_;
    [[no_unique_address]] detail::type_or_nothing<call_object_ptr, object_type*> thisptr_;
    [[no_unique_address]] detail::type_or_nothing<call_original, original_fn> original_;
    [[no_unique_address]] detail::type_or_nothing<call_original_wrapped, original_fn_wrapped> original_wrapped_;

  public:
    hook_callback_invoker_member(object_type* thisptr)
        : callback_(unique_hook_callback<Callback>)
    {
        if constexpr (store_original_and_thisptr + call_object_ptr)
            thisptr_ = thisptr;
        if constexpr (store_original_and_thisptr + call_original)
            original_ = unsafe_cast<original_fn>(unique_hook_trampoline<Callback>);
        if constexpr (call_original_wrapped)
            std::construct_at(&original_wrapped_, unique_hook_trampoline<Callback>, thisptr);
    }

    decltype(auto) operator()(Args... args) const noexcept
    {
        if constexpr (call_original_thisptr)
            return (*callback_)(original_, thisptr_, args...);
        else if constexpr (call_thisptr_original)
            return (*callback_)(thisptr_, original_, args...);
        else if constexpr (call_object_ptr)
            return (*callback_)(thisptr_, args...);
        else if constexpr (call_original)
            return (*callback_)(original_, args...);
        else if constexpr (call_original_wrapped_ex<true>)
            return (*callback_)(original_wrapped_, args...);
        else
            return (*callback_)(args...);
    }

    decltype(auto) operator()(Args... args) noexcept
    {
        if constexpr (call_original_wrapped_ex<false>)
            return (*callback_)(original_wrapped_, args...);
        else
            return std::as_const(*this)(args...);
    }
};

#define HOOK_PROXY_MEMBER(call__, __call, _call_)                                          \
    template <typename Ret, class C, typename... Args>                                     \
    struct hook_proxy_member<call__, Ret, C, Args...> final : noncopyable                  \
    {                                                                                      \
        template <typename Callback>                                                       \
        Ret __call proxy(Args... args) noexcept                                            \
        {                                                                                  \
            hook_callback_invoker_member<Callback, call__, Ret, C, Args...> invoker(this); \
            return invoker(args...);                                                       \
        }                                                                                  \
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

    Ret operator()(Args... args) const
    {
        non_member_func_invoker<Call_T, Ret, Args...> invoker;
        return invoker(original_, args...);
    }
};

template <class Callback, call_type Call_T, typename Ret, typename... Args>
class hook_callback_invoker_non_member
{
    using original_fn         = non_member_func_type<Call_T, Ret, Args...>;
    using original_fn_wrapped = object_proxy_non_member<Call_T, Ret, Args...>; // or std::bind

    template <typename... ExtraArgs>
    static constexpr bool invocable = std::invocable<Callback, ExtraArgs..., Args...>;

    static constexpr bool call_original = invocable<original_fn>;
    template <bool Const>
    static constexpr bool call_original_wrapped_ex = invocable<std::conditional_t<Const, original_fn_wrapped const&, original_fn_wrapped>>;
    static constexpr bool call_original_wrapped    = call_original_wrapped_ex<true> || call_original_wrapped_ex<false>;

#ifdef _DEBUG
    static_assert(call_original + call_original_wrapped == 1);
#endif

    Callback* callback_;
    [[no_unique_address]] detail::type_or_nothing<call_original, original_fn> original_;
    [[no_unique_address]] detail::type_or_nothing<call_original_wrapped, original_fn_wrapped> original_wrapped_;

  public:
    hook_callback_invoker_non_member()
        : callback_(unique_hook_callback<Callback>)
    {
        if constexpr (call_original)
            original_ = unsafe_cast<original_fn>(unique_hook_trampoline<Callback>);
        if constexpr (call_original_wrapped)
            std::construct_at(&original_wrapped_, unique_hook_trampoline<Callback>);
    }

    decltype(auto) operator()(Args... args) const noexcept
    {
        if constexpr (call_original)
            return (*callback_)(original_, args...);
        else if constexpr (call_original_wrapped_ex<true>)
            return (*callback_)(original_wrapped_, args...);
        else
            return (*callback_)(args...);
    }

    decltype(auto) operator()(Args... args) noexcept
    {
        if constexpr (call_original_wrapped_ex<false>)
            return (*callback_)(original_wrapped_, args...);
        else
            return std::as_const(*this)(args...);
    }
};

#define HOOK_PROXY_STATIC(call__, __call, call)                                       \
    template <typename Ret, typename... Args>                                         \
    struct hook_proxy_non_member<call__, Ret, Args...> final : noncopyable            \
    {                                                                                 \
        template <class Callback>                                                     \
        static Ret __call proxy(Args... args) noexcept                                \
        {                                                                             \
            hook_callback_invoker_non_member<Callback, call__, Ret, Args...> invoker; \
            return invoker(args...);                                                  \
        }                                                                             \
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

#if INTPTR_MAX == INT32_MAX
template <
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    typename Ret, class Object, typename... Args>
prepared_hook_data prepare_hook(Ret(__thiscall* target)(Object*, Args...))
{
    return {
        unsafe_cast<void*>(target), //
        extract_hook_proxy<Proxy<call_type::thiscall_, Ret, Object, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}
#endif

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
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = hook_proxy_non_member, //
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
struct vfunc;

template <
    typename Callback,
    HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    call_type Call_T, typename Ret, class Object, typename... Args>
prepared_hook_data prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    return {
        target.get(), //
        extract_hook_proxy<Proxy<Call_T, Ret, Object, Args...>>::template get<Callback>(),
        &unique_hook_trampoline<Callback> //
    };
}

#if 0
struct native_function_tag;

template <
    typename Callback, HOOK_PROXY_SAMPLE class Proxy = hook_proxy_member, //
    std::derived_from<native_function_tag> Fn>
prepared_hook_data prepare_hook(Fn abstract_fn)
{
    return prepare_hook<Callback, Proxy>(abstract_fn.get());
}
#endif

#undef HOOK_PROXY_SAMPLE
} // namespace fd