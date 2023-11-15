#pragma once

#include "diagnostics/fatal.h"
#include "functional/call_traits.h"
#include "hook/callback.h"
#include "hook/info.h"
#include "hook/proxy_data.h"

namespace fd
{
#define FD_HOOK_PROXY_TEMPLATE template <typename Fn>

namespace detail
{
template <typename Fn>
struct hook_proxy;

template <typename Proxy>
struct hook_proxy_target;

template <typename Fn>
struct hook_proxy_target<hook_proxy<Fn>> : std::type_identity<Fn>
{
};

template <typename Fn>
class original_func_invoker //: public noncopyable
{
    using info = function_info<Fn>;

    using object_type = typename info::object_type;

    union
    {
        Fn original_full_;
        void* original_;
    };

    union
    {
        object_type* object_;
        void const* instance_;
    };

  public:
    original_func_invoker(void* original, void const* proxy)
        : original_(original)
        , instance_(proxy)
    {
    }

    template <typename... Args>
    auto operator()(Args&&... args) const noexcept(noexcept(original_full_))
#ifdef _DEBUG
        -> std::invoke_result_t<Fn, object_type*, Args&&...>
#else
        -> typename info::return_type
#endif
    {
        return invoke(
            original_full_,
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

template <class Callback, class Proxy, typename... Args>
auto invoke_hook_proxy(Proxy* proxy, Args... args)
{
    using fn_t = typename hook_proxy_target<std::decay_t<Proxy>>::type;

    using original_invoker = original_func_invoker<fn_t>;
    using object_pointer   = typename function_info<fn_t>::object_type*;

    auto& data     = unique_hook_proxy_data<Callback>;
    auto& callback = *data.callback;

    if constexpr (std::invocable<Callback, original_invoker, Args...>)
        return callback(original_invoker(data.original, proxy), args...);
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

template <class Callback, class Fn, typename... Args>
auto invoke_hook_proxy(Args... args)
{
    using original_invoker = Fn;

    auto& data     = unique_hook_proxy_data<Callback>;
    auto& callback = *data.callback;

    if constexpr (std::invocable<Callback, original_invoker, Args...>)
        return callback(original_invoker(data.original), args...);
    else
        return callback(args...);
}

#define HOOK_PROXY_STATIC(_CCV_, _CV_UNUSED_, _REF_UNUSED_, _NOEXCEPT_)                            \
    template <typename Ret, typename... Args>                                                      \
    struct hook_proxy<Ret(_CCV_*)(Args...) _NOEXCEPT_> : noncopyable                               \
    {                                                                                              \
        template <class Callback>                                                                  \
        static Ret _CCV_ proxy(Args... args) _NOEXCEPT_                                            \
        {                                                                                          \
            return invoke_hook_proxy<Callback, Ret(_CCV_*)(Args...) _NOEXCEPT_, Args...>(args...); \
        }                                                                                          \
    };

#ifdef _MSC_VER
_MEMBER_CALL_CV_REF_NOEXCEPT(HOOK_PROXY_MEMBER)
_NON_MEMBER_CALL(HOOK_PROXY_STATIC, , , );
_NON_MEMBER_CALL(HOOK_PROXY_STATIC, , , noexcept);
#else

#endif
} // namespace detail

template <class Callback, class Proxy, typename Func>
hook_info<Callback> prepare_hook(Func fn)
{
    return {unsafe_cast<void*>(fn), unsafe_cast<void*>(&Proxy::template proxy<Callback>)};
}

template <class Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info<Callback> prepare_hook(Func fn)
#ifdef _DEBUG
    requires(complete<Proxy<Func>>)
#endif
{
    return prepare_hook<Callback, Proxy<Func>>(fn);
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

template <class Call_T, typename Ret, class Object, typename... Args>
struct vfunc;

template <
    typename Callback,
    FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, //
    class Call_T,
    typename Ret,
    class Object,
    typename... Args>
hook_info<Callback> prepare_hook(vfunc<Call_T, Ret, Object, Args...> target)
{
    return prepare_hook<Callback, Proxy>(get(target));
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
} // namespace fd