#pragma once

#include "hook.h"

#include <fd/magic_cast.h>
#include <fd/x86_call.h>

#include <optional>

namespace fd
{
template <typename Callback>
std::optional<Callback> hook_callback_stored;

template <typename Callback>
void *hook_trampoline_stored;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member<Callback &, Call, Ret, C, Args...>
    : hook_callback_proxy_member<Callback, Call, Ret, C, Args...>
{
};

#define HOOK_CALLBACK_PROXY_MEMBER(call__, __call, call)                            \
    template <class Callback, typename Ret, class C, typename... Args>              \
    struct hook_callback_proxy_member<Callback, call__, Ret, C, Args...>            \
    {                                                                               \
        using original_type = Ret (__call hook_callback_proxy_member::*)(Args...);  \
        Ret __call proxy(Args... args) noexcept                                     \
        {                                                                           \
            original_type fn = to<original_type>(hook_trampoline_stored<Callback>); \
            auto &callback   = *hook_callback_stored<Callback>;                     \
            return callback(                                                        \
                [&](Args... proxy_args) -> Ret {                                    \
                    return (*this.*fn)(static_cast<Args>(proxy_args)...); /**/      \
                },                                                                  \
                reinterpret_cast<C *>(this),                                        \
                std::forward<Args>(args)...);                                       \
        }                                                                           \
    };

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy;

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy<Callback &, Call, Ret, Args...> : hook_callback_proxy<Callback, Call, Ret, Args...>
{
};

template <class Callback, typename Ret, typename C, typename... Args>
struct hook_callback_proxy<Callback, _x86_call::thiscall__, Ret, C, Args...>
    : hook_callback_proxy_member<Callback, _x86_call::thiscall__, Ret, C, Args...>
{
};

#define HOOK_CALLBACK_PROXY_STATIC(call__, __call, call)                  \
    template <class Callback, typename Ret, typename... Args>             \
    struct hook_callback_proxy<Callback, call__, Ret, Args...>            \
    {                                                                     \
        using function_type = Ret(__call *)(Args...);                     \
        static Ret __call proxy(Args... args) noexcept                    \
        {                                                                 \
            auto &callback = *hook_callback_stored<Callback>;             \
            return callback(                                              \
                to<function_type>(hook_trampoline_stored<Callback>), /**/ \
                std::forward<Args>(args)...);                             \
        }                                                                 \
    };

X86_CALL(HOOK_CALLBACK_PROXY_STATIC);
#undef HOOK_CALLBACK_PROXY_STATIC
X86_CALL_MEMBER(HOOK_CALLBACK_PROXY_MEMBER);
#undef HOOK_CALLBACK_PROXY_MEMBER

template <typename Proxy, typename Callback>
hook_id init_hook_callback(hook_name name, void *target, Callback &callback)
{
    //[[maybe_unused]]//
    // static Proxy proxy;

#ifdef _DEBUG
    static auto stored_target = target;
    if (stored_target != target)
        std::terminate();
#endif

    auto id = create_hook(target, to<void *>(&Proxy::proxy), name, &hook_trampoline_stored<Callback>);
    if (id)
        hook_callback_stored<Callback>.emplace(std::move(callback));
    return id;
}

#define MAKE_HOOK_CALLBACK(call__, __call, call)                                                             \
    template <typename Callback, typename Ret, class C, typename... Args>                                    \
    hook_id make_hook_callback(hook_name name, Ret (__call C::*target)(Args...), Callback callback) noexcept \
    {                                                                                                        \
        using proxy_type = hook_callback_proxy_member<Callback, call__, Ret, C, Args...>;                    \
        return init_hook_callback<proxy_type>(name, to<void *>(target), callback);                           \
    }                                                                                                        \
    template <typename Callback, typename Ret, typename... Args>                                             \
    hook_id make_hook_callback(hook_name name, Ret(__call *target)(Args...), Callback callback) noexcept     \
    {                                                                                                        \
        using proxy_type = hook_callback_proxy<Callback, call__, Ret, Args...>;                              \
        return init_hook_callback<proxy_type>(name, to<void *>(target), callback);                           \
    }

X86_CALL_MEMBER(MAKE_HOOK_CALLBACK);
#undef MAKE_HOOK_CALLBACK

template <_x86_call Call, typename Ret, typename T, typename... Args>
class vfunc;

template <typename Callback, _x86_call Call, typename Ret, typename T, typename... Args>
hook_id make_hook_callback(hook_name name, vfunc<Call, Ret, T, Args...> target, Callback callback)
{
    using proxy_type = hook_callback_proxy_member<Callback, Call, Ret, T, Args...>;
    return init_hook_callback<proxy_type>(name, target.get(), callback);
}

template <typename Callback, typename From, typename Sample>
hook_id make_hook_callback(hook_name name, magic_cast<From, Sample> target, Callback callback)
{
    return make_hook_callback(name, static_cast<Sample>(target), callback);
}

} // namespace fd