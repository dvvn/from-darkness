#pragma once

#include "hook.h"

#include <fd/magic_cast.h>
#include <fd/x86_call.h>

#include <boost/core/noncopyable.hpp>

#include <optional>

namespace fd
{
template <typename Callback>
std::optional<Callback> hook_callback_stored;

template <typename Callback>
void *hook_trampoline_stored;

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
struct hook_proxy_member;

// template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
// struct hook_proxy_member<Callback &, Call, Ret, C, Args...>
//     : hook_proxy_member<Callback, Call, Ret, C, Args...>
//{
// };

template <class Callback, _x86_call Call, typename Ret, class C, typename... Args>
class hook_proxy_member_holder;

template <typename Callback, typename Unwrapped, bool = std::convertible_to<Callback, Unwrapped>>
struct try_unwrap_hook_callback;

template <typename Callback, typename Unwrapped>
struct try_unwrap_hook_callback<Callback, Unwrapped, true>
{
    using type = Unwrapped;
};

template <typename Callback, typename Unwrapped>
struct try_unwrap_hook_callback<Callback, Unwrapped, false>
{
    using type = Callback &;
};

struct hook_proxy_member_original_gap
{
};

#define HOOK_PROXY_MEMBER(call__, __call, call)                                                               \
    template <class Callback, typename Ret, class C, typename... Args>                                        \
    class hook_proxy_member_holder<Callback, call__, Ret, C, Args...>                                         \
    {                                                                                                         \
        union                                                                                                 \
        {                                                                                                     \
            void *proxy_;                                                                                     \
            C *this_;                                                                                         \
            hook_proxy_member_original_gap *gap_;                                                             \
        };                                                                                                    \
                                                                                                              \
        Ret (__call hook_proxy_member_original_gap::*original_)(Args...);                                     \
                                                                                                              \
      public:                                                                                                 \
        hook_proxy_member_holder(void *proxy)                                                                 \
            : proxy_(proxy)                                                                                   \
            , original_(magic_cast(hook_trampoline_stored<Callback>, original_))                              \
        {                                                                                                     \
        }                                                                                                     \
        Ret __call operator()(Args... args) noexcept                                                          \
        {                                                                                                     \
            return (*gap_.*original_)(args...);                                                               \
        }                                                                                                     \
        operator C *()                                                                                        \
        {                                                                                                     \
            return this_;                                                                                     \
        }                                                                                                     \
        C *operator->()                                                                                       \
        {                                                                                                     \
            return this_;                                                                                     \
        }                                                                                                     \
    };                                                                                                        \
    template <class Callback, typename Ret, class C, typename... Args>                                        \
    struct hook_proxy_member<Callback, call__, Ret, C, Args...> : boost::noncopyable                          \
    {                                                                                                         \
        Ret __call operator()(Args... args) noexcept                                                          \
        {                                                                                                     \
            using original_type                 = Ret (__call hook_proxy_member_original_gap::*)(Args...);    \
            hook_proxy_member_original_gap *gap = to<hook_proxy_member_original_gap *>(this);                 \
            original_type original              = to<original_type>(hook_trampoline_stored<Callback>);        \
            return (*gap.*original)(args...);                                                                 \
        }                                                                                                     \
        operator C *()                                                                                        \
        {                                                                                                     \
            return to<C *>(this);                                                                             \
        }                                                                                                     \
        C *operator->()                                                                                       \
        {                                                                                                     \
            return to<C *>(this);                                                                             \
        }                                                                                                     \
        Ret __call proxy(Args... args) noexcept                                                               \
        {                                                                                                     \
            using callback_unwrapped = Ret(__call *)(hook_proxy_member &, Args...);                           \
            using callback_type      = typename try_unwrap_hook_callback<Callback, callback_unwrapped>::type; \
            using proxy_holder       = hook_proxy_member_holder<Callback, call__, Ret, C, Args...>;           \
            callback_type callback   = *hook_callback_stored<Callback>;                                       \
            if constexpr (std::invocable<callback_type, hook_proxy_member &, Args...>)                        \
                return callback(*this, args...);                                                              \
            else                                                                                              \
                return callback(proxy_holder(this), args...);                                                 \
        }                                                                                                     \
    };

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_proxy;

template <class Callback, _x86_call Call, typename Ret, typename... Args>
struct hook_proxy<Callback &, Call, Ret, Args...> : hook_proxy<Callback, Call, Ret, Args...>
{
};

template <class Callback, typename Ret, typename C, typename... Args>
struct hook_proxy<Callback, _x86_call::thiscall__, Ret, C, Args...>
    : hook_proxy_member<Callback, _x86_call::thiscall__, Ret, C, Args...>
{
};

#define HOOK_PROXY_STATIC(call__, __call, call)                                                                \
    template <class Callback, typename Ret, typename... Args>                                                  \
    struct hook_proxy<Callback, call__, Ret, Args...>                                                          \
    {                                                                                                          \
        static Ret __call proxy(Args... args) noexcept                                                         \
        {                                                                                                      \
            using function_type      = Ret(__call *)(Args...);                                                 \
            using callback_unwrapped = Ret(__call *)(function_type, Args...);                                  \
            using callback_type      = typename try_unwrap_hook_callback<Callback, callback_unwrapped>::type;  \
            callback_type callback   = *hook_callback_stored<Callback>;                                        \
            return callback(to<function_type>(hook_trampoline_stored<Callback>), std::forward<Args>(args)...); \
        }                                                                                                      \
    };

X86_CALL(HOOK_PROXY_STATIC);
#undef HOOK_PROXY_STATIC
X86_CALL_MEMBER(HOOK_PROXY_MEMBER);
#undef HOOK_PROXY_MEMBER

inline hook_id create_hook(void *target, to<void *> replace, hook_name name, void **trampoline)
{
    return create_hook(target, static_cast<void *>(replace), name, trampoline);
}

template <typename Proxy, typename Callback>
hook_id init_hook_callback(hook_name name, to<void *> target, Callback &callback)
{
    //[[maybe_unused]]//
    // static Proxy proxy;

#ifdef _DEBUG
    static auto stored_target = target;
#ifdef assert
    assert(stored_target == target);
#else
    if (stored_target != target)
        std::terminate();
#endif
#endif

    auto id = create_hook(target, &Proxy::proxy, name, &hook_trampoline_stored<Callback>);
    if (id)
        hook_callback_stored<Callback>.emplace(std::move(callback));
    return id;
}

#define MAKE_HOOK_CALLBACK(call__, __call, call)                                                             \
    template <typename Callback, typename Ret, class C, typename... Args>                                    \
    hook_id make_hook_callback(hook_name name, Ret (__call C::*target)(Args...), Callback callback) noexcept \
    {                                                                                                        \
        using proxy_type = hook_proxy_member<Callback, call__, Ret, C, Args...>;                             \
        return init_hook_callback<proxy_type>(name, target, callback);                                       \
    }                                                                                                        \
    template <typename Callback, typename Ret, typename... Args>                                             \
    hook_id make_hook_callback(hook_name name, Ret(__call *target)(Args...), Callback callback) noexcept     \
    {                                                                                                        \
        using proxy_type = hook_proxy<Callback, call__, Ret, Args...>;                                       \
        return init_hook_callback<proxy_type>(name, target, callback);                                       \
    }

X86_CALL_MEMBER(MAKE_HOOK_CALLBACK);
#undef MAKE_HOOK_CALLBACK

template <_x86_call Call, typename Ret, typename T, typename... Args>
class vfunc;

template <typename Callback, _x86_call Call, typename Ret, typename T, typename... Args>
hook_id make_hook_callback(hook_name name, vfunc<Call, Ret, T, Args...> target, Callback callback)
{
    using proxy_type = hook_proxy_member<Callback, Call, Ret, T, Args...>;
    return init_hook_callback<proxy_type>(name, target.get(), callback);
}

template <typename Callback, typename From, typename Sample>
hook_id make_hook_callback(hook_name name, magic_cast<From, Sample> target, Callback callback)
{
    return make_hook_callback(name, static_cast<Sample>(target), callback);
}

} // namespace fd