#pragma once

#include "diagnostics/fatal.h"
#include "functional/basic_vfunc.h"
#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"

#define FD_HOOK_PROXY_TEMPLATE template <typename Fn>

namespace fd
{
namespace detail
{
template <typename Fn>
struct hook_proxy;

template <typename Fn>
#ifdef _DEBUG
requires requires { typename function_info<Fn>::object_type; }
#endif
class member_func_invoker : public noncopyable
{
    using fn_info = function_info<Fn>;

    using object_type = typename fn_info::object_type;

    using object_pointer   = object_type*;
    using instance_pointer = safe_cast_result<void*, object_pointer>;

    Fn original_;

    union
    {
        object_pointer object_;
        instance_pointer instance_;
    };

  public:
    member_func_invoker(void* original, instance_pointer proxy)
        : original_{unsafe_cast_from(original)}
        , instance_{proxy}
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(fn_info::no_throw)
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, object_pointer, Args&&...>
#else
        -> decltype(auto)
#endif
    {
        return invoke(
            original_,
#if defined(FD_SPOOF_RETURN_ADDRESS) || defined(_DEBUG)
            object_,
#else
            instance_,
#endif
            std::forward<Args>(args)...);
    }

    operator object_pointer() const
    {
        return object_;
    }

    object_pointer operator->() const
    {
        return object_;
    }
};

template <typename Fn>
class non_member_func_invoker : public noncopyable
{
    Fn original_;

  public:
    non_member_func_invoker(void* original)
        : original_{unsafe_cast_from(original)}
    {
    }

    non_member_func_invoker(Fn original)
        : original_{original}
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(function_info<Fn>::no_throw)
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, Args&&...>
#else
        -> decltype(auto)
#endif
    {
        using std::invoke;
#ifdef FD_SPOOF_RETURN_ADDRESS
        // WIP
#else
        return invoke(original_, std::forward<Args>(args)...);
#endif
    }
};

template <typename Callback, class OriginalInvoker, class ObjectPointer, class Proxy, typename... Args>
decltype(auto) invoke_hook_proxy_member(Proxy* proxy, Args&&... args)
{
    auto& callback = *unique_hook_proxy_data<Callback>.callback;

    if constexpr (std::invocable<Callback, OriginalInvoker, Args&&...>)
        return invoke_hook_callback(callback, OriginalInvoker{unique_hook_proxy_data<Callback>.original, proxy}, std::forward<Args>(args)...);
    else if constexpr (std::invocable<Callback, ObjectPointer, Args&&...>)
        return invoke_hook_callback(callback, unsafe_cast<ObjectPointer>(proxy), std::forward<Args>(args)...);
    else if constexpr (std::invocable<Callback, ObjectPointer>)
        return invoke_hook_callback(callback, unsafe_cast<ObjectPointer>(proxy));
    else
        return invoke_hook_callback(callback, std::forward<Args>(args)...);
}

template <typename Callback, class Func, typename... Args>
decltype(auto) invoke_hook_proxy(hook_proxy<Func>* proxy, Args&&... args)
{
    return invoke_hook_proxy_member<Callback, member_func_invoker<Func>, typename function_info<Func>::object_type*>(proxy, std::forward<Args>(args)...);
}

template <typename Callback, class Func, typename... Args>
decltype(auto) invoke_hook_proxy(hook_proxy<Func> const* proxy, Args&&... args)
{
    return invoke_hook_proxy_member<Callback, member_func_invoker<Func>, typename function_info<Func>::object_type*>(proxy, std::forward<Args>(args)...);
}

#define HOOK_PROXY_MEMBER(_CCV_, _CV_, _REF_, _NOEXCEPT_)                                 \
    template <typename Ret, class Object, typename... Args>                               \
    struct hook_proxy<Ret (_CCV_ Object::*)(Args...) _CV_ _REF_ _NOEXCEPT_> : noncopyable \
    {                                                                                     \
        template <typename Callback>                                                      \
        Ret _CCV_ proxy(Args... args) _CV_ _REF_ _NOEXCEPT_                               \
        {                                                                                 \
            return invoke_hook_proxy<Callback>(this, args...);                            \
        }                                                                                 \
    };

template <typename Callback, typename Fn, typename... Args>
decltype(auto) invoke_hook_proxy(type_identity<Fn> /*target*/, Args&&... args)
{
    using original_invoker = non_member_func_invoker<Fn>;

    auto& callback = *unique_hook_proxy_data<Callback>.callback;

    if constexpr (std::invocable<Callback, original_invoker, Args...>)
        return invoke_hook_callback(callback, original_invoker{unique_hook_proxy_data<Callback>.original}, std::forward<Args>(args)...);
    else
        return invoke_hook_callback(callback, std::forward<Args>(args)...);
}

#define HOOK_PROXY_STATIC(_CCV_, _CV_UNUSED_, _REF_UNUSED_, _NOEXCEPT_)                                    \
    template <typename Ret, typename... Args>                                                              \
    struct hook_proxy<Ret(_CCV_*)(Args...) _NOEXCEPT_> : noncopyable                                       \
    {                                                                                                      \
        template <typename Callback>                                                                       \
        static Ret _CCV_ proxy(Args... args) _NOEXCEPT_                                                    \
        {                                                                                                  \
            return invoke_hook_proxy<Callback>(type_identity<Ret(_CCV_*)(Args...) _NOEXCEPT_>{}, args...); \
        }                                                                                                  \
    };

#define HOOK_PROXY_STATIC_THISCALL(_CCV_, _CV_, _REF_, _NOEXCEPT_)                        \
    template <typename Ret, class Object, typename... Args>                               \
    struct hook_proxy<Ret(_CCV_*)(Object _CV_ * _REF_, Args...) _NOEXCEPT_> : noncopyable \
    {                                                                                     \
        template <typename Callback>                                                      \
        Ret _CCV_ proxy(Args... args) _CV_ _REF_ _NOEXCEPT_                               \
        {                                                                                 \
            return invoke_hook_proxy<Callback>(this, args...);                            \
        }                                                                                 \
    };

#ifdef _MSC_VER
_MEMBER_CALL_CV_REF_NOEXCEPT(HOOK_PROXY_MEMBER)
_NON_MEMBER_CALL(HOOK_PROXY_STATIC, , , );
_NON_MEMBER_CALL(HOOK_PROXY_STATIC, , , noexcept);
_NON_MEMBER_CALL_THISCALL_CV_REF_NOEXCEPT(HOOK_PROXY_STATIC_THISCALL) // macro from functional/call_traits.h
#else

#endif

#undef HOOK_PROXY_STATIC_THISCALL
#undef HOOK_PROXY_STATIC
#undef HOOK_PROXY_MEMBER

} // namespace detail

template <typename Callback, class Proxy, typename Func, bool Inner = false>
hook_info<Callback> prepare_hook(Func const fn, bool_constant<Inner> = {}) requires(Inner || complete<Proxy>)
{
    return {unsafe_cast<void*>(fn), unsafe_cast<void*>(&Proxy::template proxy<Callback>)};
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info<Callback> prepare_hook(Func const fn) requires(complete<Proxy<Func>>)
{
    return prepare_hook<Callback, Proxy<Func>>(fn, true_type{});
}

template <typename Callback, class Proxy, typename Func>
hook_info<Callback> prepare_hook(vfunc<Func> const target)
{
    return prepare_hook<Callback, Proxy, Func>(target, true_type{});
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info<Callback> prepare_hook(vfunc<Func> const target)
{
    return prepare_hook<Callback, Proxy<Func>, Func>(target, true_type{});
}
} // namespace fd