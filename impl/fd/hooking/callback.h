#pragma once

#include <fd/hooking/hook.h>
#include <fd/magic_cast.h>
#include <fd/manual_construct.h>

namespace fd
{
#undef cdecl
// ReSharper disable CppInconsistentNaming
enum class _x86_call : uint8_t
{
    thiscall_,
    cdecl_,
    fastcall_,
    stdcall_,
    vectorcall_,
};

template <typename Callback>
manual_construct_t<Callback> hook_callback_stored;

template <typename Callback>
void *hook_trampoline_stored;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member;

#define HOOK_CALLBACK_PROXY_MEMBER(_CALL_)                                                   \
    template <class Callback, typename Ret, class C, typename... Args>                       \
    struct hook_callback_proxy_member<Callback, _x86_call::_CALL_##_, Ret, C, Args...> final \
    {                                                                                        \
        using original_type = Ret (__##_CALL_ hook_callback_proxy_member::*)(Args...);       \
        Ret __##_CALL_ proxy(Args... args) noexcept                                          \
        {                                                                                    \
            original_type fn = from_void<original_type>(hook_trampoline_stored<Callback>);   \
            return extract_manual_object(hook_callback_stored<Callback>)(                    \
                [&](Args... proxy_args) -> Ret {                                             \
                    /**/                                                                     \
                    return (*this.*fn)(static_cast<Args>(proxy_args)...);                    \
                },                                                                           \
                reinterpret_cast<C *>(this),                                                 \
                std::forward<Args>(args)...);                                                \
        }                                                                                    \
    };

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy;

template <class Callback, typename Ret, typename... Args>
struct hook_callback_proxy<Callback, _x86_call::thiscall_, Ret, Args...>
{
};

#define HOOK_CALLBACK_PROXY_STATIC(_CALL_)                                         \
    template <class Callback, typename Ret, typename... Args>                      \
    struct hook_callback_proxy<Callback, _x86_call::_CALL_##_, Ret, Args...> final \
    {                                                                              \
        using function_type = Ret(__##_CALL_ *)(Args...);                          \
        static Ret __##_CALL_ proxy(Args... args) noexcept                         \
        {                                                                          \
            return extract_manual_object(hook_callback_stored<Callback>)(          \
                from_void<function_type>(hook_trampoline_stored<Callback>), /**/   \
                std::forward<Args>(args)...);                                      \
        }                                                                          \
    };

#define HOOK_CALLBACK_PROXY_ANY(_CALL_) \
    HOOK_CALLBACK_PROXY_STATIC(_CALL_)  \
    HOOK_CALLBACK_PROXY_MEMBER(_CALL_)

HOOK_CALLBACK_PROXY_ANY(cdecl);
HOOK_CALLBACK_PROXY_ANY(fastcall);
HOOK_CALLBACK_PROXY_ANY(stdcall);
HOOK_CALLBACK_PROXY_ANY(vectorcall);
HOOK_CALLBACK_PROXY_MEMBER(thiscall);

using raw_hook_name = char const *;

template <typename HookProxy, typename Callback>
basic_hook *init_hook_callback(raw_hook_name name, void *target, Callback &replace)
{
    //[[maybe_unused]] //
    // static HookProxy proxy;
    static auto holder = hook(name);

    if (!holder.init(target, to_void(&HookProxy::proxy)))
        return nullptr;

    auto &callback   = hook_callback_stored<Callback>;
    auto &trampoline = hook_trampoline_stored<Callback>;

    construct_manual_object(callback, std::move(replace));
    trampoline = holder.get_original_method();

    return &holder;
}

#define HOOK_CALLBACK_MEMBER(_CALL_)                                                                                  \
    template <typename Callback, typename Ret, class C, typename... Args>                                             \
    basic_hook *make_hook_callback(raw_hook_name name, Ret (__##_CALL_ C::*target)(Args...), Callback replace)        \
    {                                                                                                                 \
        using proxy_type = hook_callback_proxy_member<std::decay_t<Callback>, _x86_call::_CALL_##_, Ret, C, Args...>; \
        return init_hook_callback<proxy_type>(name, to_void(target), replace);                                        \
    }

#define HOOK_CALLBACK_STATIC(_CALL_)                                                                                   \
    template <typename Callback, typename Ret, typename... Args>                                                       \
    basic_hook *make_hook_callback(raw_hook_name name, Ret(__##_CALL_ *target)(Args...), Callback replace)             \
    {                                                                                                                  \
        using proxy_type = hook_callback_proxy<std::remove_reference_t<Callback>, _x86_call::_CALL_##_, Ret, Args...>; \
        return init_hook_callback<proxy_type>(name, to_void(target), replace);                                         \
    }

#define HOOK_CALLBACK_ANY(_CALL_) \
    HOOK_CALLBACK_STATIC(_CALL_)  \
    HOOK_CALLBACK_MEMBER(_CALL_)

HOOK_CALLBACK_ANY(cdecl);
HOOK_CALLBACK_ANY(fastcall);
HOOK_CALLBACK_ANY(stdcall);
HOOK_CALLBACK_ANY(vectorcall);
HOOK_CALLBACK_MEMBER(thiscall);

template <typename Callback, typename Sample>
basic_hook *make_hook_callback(raw_hook_name name, from_void<Sample> target, Callback replace)
{
    return make_hook_callback<Callback &>(name, static_cast<Sample>(target), replace);
}

} // namespace fd