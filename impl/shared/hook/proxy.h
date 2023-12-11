#pragma once

#include "diagnostics/fatal.h"
#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"

namespace fd
{
template <typename Fn, size_t FnSize>
struct vfunc;
}

#define FD_HOOK_PROXY_TEMPLATE template <typename Fn>

namespace fd
{
namespace detail
{
template <typename Fn>
struct hook_proxy;

template <typename Proxy>
struct hook_proxy_target;

template <typename Fn>
struct hook_proxy_target<hook_proxy<Fn>> : type_identity<Fn>
{
};

template <typename Fn>
struct hook_proxy_target<hook_proxy<Fn> const> : type_identity<Fn>
{
};

template <typename Fn>
struct hook_proxy_target<hook_proxy<Fn> volatile> : type_identity<Fn>
{
};

template <typename Fn>
struct hook_proxy_target<hook_proxy<Fn> volatile const> : type_identity<Fn>
{
};

template <typename Fn, bool Member>
class original_func_invoker;

template <typename Fn>
#ifdef _DEBUG
requires requires { typename function_info<Fn>::object_type; }
#endif
class original_func_invoker<Fn, true> : public noncopyable
{
    using info = function_info<Fn>;

    using object_type   = typename info::object_type;
    using instance_type = conditional_t<std::is_const_v<object_type>, void const, void>;

    Fn original_;

    union
    {
        object_type* object_;
        instance_type* instance_;
    };

  public:
    original_func_invoker(void* original, instance_type* proxy)
        : original_{unsafe_cast_from(original)}
        , instance_{proxy}
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(info::no_throw)
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, object_type*, Args&&...>
#else
        -> typename info::return_type
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

    operator object_type*() const
    {
        return object_;
    }

    object_type* operator->() const
    {
        return object_;
    }
};

template <typename Fn>
class original_func_invoker<Fn, false> : public noncopyable
{
    using info = function_info<Fn>;

    Fn original_;

  public:
    original_func_invoker(void* original)
        : original_{unsafe_cast_from(original)}
    {
    }

    original_func_invoker(Fn original)
        : original_{original}
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(info::no_throw)
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, Args&&...>
#else
        -> typename info::return_type
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

template <typename Fn, class Object>
original_func_invoker(Fn, Object*) -> original_func_invoker<Fn, true>;

template <typename Fn>
original_func_invoker(Fn) -> original_func_invoker<Fn, false>;

template <typename Callback, class Proxy, typename... Args>
auto invoke_hook_proxy(Proxy* proxy, Args... args)
{
    using fn_t = typename hook_proxy_target<Proxy>::type;

    using original_invoker = original_func_invoker<fn_t, true>;
    using object_pointer   = typename function_info<fn_t>::object_type*;

    decltype(auto) callback = make_hook_callback_invoker(unique_hook_proxy_data<Callback>.callback);

    if constexpr (std::invocable<Callback, original_invoker, Args...>)
        return callback(original_invoker{unique_hook_proxy_data<Callback>.original, proxy}, args...);
    else if constexpr (std::invocable<Callback, object_pointer, Args...>)
        return callback(unsafe_cast<object_pointer>(proxy), args...);
    else if constexpr (std::invocable<Callback, object_pointer>)
        return callback(unsafe_cast<object_pointer>(proxy), args...);
    else
        return callback(args...);
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
auto invoke_hook_proxy(type_identity<Fn> /*target*/, Args... args)
{
    using original_invoker = original_func_invoker<Fn, false>;

    decltype(auto) callback = make_hook_callback_invoker(unique_hook_proxy_data<Callback>.callback);

    if constexpr (std::invocable<Callback, original_invoker, Args...>)
        return callback(original_invoker{unique_hook_proxy_data<Callback>.original}, args...);
    else
        return callback(args...);
}

#define HOOK_PROXY_STATIC(_CCV_, _CV_UNUSED_, _REF_UNUSED_, _NOEXCEPT_)                   \
    template <typename Ret, typename... Args>                                             \
    struct hook_proxy<Ret(_CCV_*)(Args...) _NOEXCEPT_> : noncopyable                      \
    {                                                                                     \
        template <typename Callback>                                                      \
        static Ret _CCV_ proxy(Args... args) _NOEXCEPT_                                   \
        {                                                                                 \
            return invoke_hook_proxy<Callback>(hook_proxy_target<hook_proxy>{}, args...); \
        }                                                                                 \
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
hook_info<Callback> prepare_hook(Func fn, bool_constant<Inner> = {}) requires(Inner || complete<Proxy>)
{
    return {unsafe_cast<void*>(fn), unsafe_cast<void*>(&Proxy::template proxy<Callback>)};
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info<Callback> prepare_hook(Func fn) requires(complete<Proxy<Func>>)
{
    return prepare_hook<Callback, Proxy<Func>>(fn, true_type{});
}

template <typename Callback, class Proxy, typename Func, size_t FuncSize>
hook_info<Callback> prepare_hook(vfunc<Func, FuncSize> target)
{
    return prepare_hook<Callback, Proxy, Func>(target, true_type{});
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func, size_t FuncSize>
hook_info<Callback> prepare_hook(vfunc<Func, FuncSize> target)
{
    return prepare_hook<Callback, Proxy<Func>, Func>(target, true_type{});
}
} // namespace fd