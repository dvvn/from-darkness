#pragma once

#include <fd/hooking/hook.h>
#include <fd/magic_cast.h>

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
Callback *hook_callback_stored;

template <typename Callback>
void *hook_trampoline_stored;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_callback_proxy_member<Callback &, Call, Ret, C, Args...>
    : hook_callback_proxy_member<Callback, Call, Ret, C, Args...>
{
};

#define HOOK_CALLBACK_PROXY_MEMBER(_CALL_)                                             \
    template <class Callback, typename Ret, class C, typename... Args>                 \
    struct hook_callback_proxy_member<Callback, _x86_call::_CALL_##_, Ret, C, Args...> \
    {                                                                                  \
        using original_type = Ret (__##_CALL_ hook_callback_proxy_member::*)(Args...); \
        Ret __##_CALL_ proxy(Args... args) noexcept                                    \
        {                                                                              \
            original_type fn = to<original_type>(hook_trampoline_stored<Callback>);    \
            auto &callback   = *hook_callback_stored<Callback>;                        \
            return callback(                                                           \
                [&](Args... proxy_args) -> Ret {                                       \
                    return (*this.*fn)(static_cast<Args>(proxy_args)...); /**/         \
                },                                                                     \
                reinterpret_cast<C *>(this),                                           \
                std::forward<Args>(args)...);                                          \
        }                                                                              \
    };

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy;

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_callback_proxy<Callback &, Call, Ret, Args...> : hook_callback_proxy<Callback, Call, Ret, Args...>
{
};

template <class Callback, typename Ret, typename C, typename... Args>
struct hook_callback_proxy<Callback, _x86_call::thiscall_, Ret, C, Args...>
    : hook_callback_proxy_member<Callback, _x86_call::thiscall_, Ret, C, Args...>
{
};

#define HOOK_CALLBACK_PROXY_STATIC(_CALL_)                                   \
    template <class Callback, typename Ret, typename... Args>                \
    struct hook_callback_proxy<Callback, _x86_call::_CALL_##_, Ret, Args...> \
    {                                                                        \
        using function_type = Ret(__##_CALL_ *)(Args...);                    \
        static Ret __##_CALL_ proxy(Args... args) noexcept                   \
        {                                                                    \
            auto &callback = *hook_callback_stored<Callback>;                \
            return callback(                                                 \
                to<function_type>(hook_trampoline_stored<Callback>), /**/    \
                std::forward<Args>(args)...);                                \
        }                                                                    \
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

template <typename Proxy, typename Callback>
basic_hook *init_hook_callback(raw_hook_name name, void *target, Callback &callback)
{
    //[[maybe_unused]]//
    // static Proxy proxy;
#ifdef _DEBUG
    auto first_time    = false;
    static auto holder = (first_time = true, hook(name));
    if (!first_time)
        std::terminate();
#else
    static auto holder = hook(name);
#endif

    if (!holder.init(target, to<void *>(&Proxy::proxy)))
        return nullptr;

    static auto callback_stored = std::move(callback);
    auto trampoline             = holder.get_original_method();

    hook_callback_stored<Callback>   = &callback_stored;
    hook_trampoline_stored<Callback> = trampoline;

    return &holder;
}

#define HOOK_CALLBACK_MEMBER(_CALL_)                                                                    \
    template <typename Callback, typename Ret, class C, typename... Args>                               \
    basic_hook *make_hook_callback(                                                                     \
        raw_hook_name name, Ret (__##_CALL_ C::*target)(Args...), Callback callback) noexcept           \
    {                                                                                                   \
        using proxy_type = hook_callback_proxy_member<Callback, _x86_call::_CALL_##_, Ret, C, Args...>; \
        return init_hook_callback<proxy_type>(name, to<void *>(target), callback);                      \
    }

#define HOOK_CALLBACK_STATIC(_CALL_)                                                                                 \
    template <typename Callback, typename Ret, typename... Args>                                                     \
    basic_hook *make_hook_callback(raw_hook_name name, Ret(__##_CALL_ *target)(Args...), Callback callback) noexcept \
    {                                                                                                                \
        using proxy_type = hook_callback_proxy<Callback, _x86_call::_CALL_##_, Ret, Args...>;                        \
        return init_hook_callback<proxy_type>(name, to<void *>(target), callback);                                   \
    }

#define HOOK_CALLBACK_ANY(_CALL_) \
    HOOK_CALLBACK_STATIC(_CALL_)  \
    HOOK_CALLBACK_MEMBER(_CALL_)

HOOK_CALLBACK_ANY(cdecl);
HOOK_CALLBACK_ANY(fastcall);
HOOK_CALLBACK_ANY(stdcall);
HOOK_CALLBACK_ANY(vectorcall);
HOOK_CALLBACK_ANY(thiscall);

#undef HOOK_CALLBACK_STATIC
#undef HOOK_CALLBACK_ANY
#undef HOOK_CALLBACK_MEMBER

template <typename Callback>
using try_add_ref = std::conditional_t<
    sizeof(Callback) <= sizeof(uintptr_t[2]) &&   //
        std::is_trivially_copyable_v<Callback> && //
        std::is_trivially_destructible_v<Callback>,
    Callback,
    std::add_lvalue_reference_t<Callback>>;

template <typename Callback, typename From, typename Sample>
basic_hook *make_hook_callback(raw_hook_name name, magic_cast<From, Sample> target, Callback callback)
{
    return make_hook_callback<try_add_ref<Callback>>(name, static_cast<Sample>(target), callback);
}

} // namespace fd