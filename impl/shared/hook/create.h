#pragma once

#include "hook/proxy.h"

namespace fd
{
struct hook_info
{
    void* target;
    void* replace;
    // void* original;
};

template <typename Callback, class Proxy, typename Func, bool Inner = false>
hook_info prepare_hook(Func const fn, std::bool_constant<Inner> = {}) requires(Inner || complete<Proxy>)
{
    return {unsafe_cast<void*>(fn), unsafe_cast<void*>(&Proxy::template proxy<Callback>)};
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info prepare_hook(Func const fn) requires(complete<Proxy<Func>>)
{
    return prepare_hook<Callback, Proxy<Func>>(fn, std::true_type{});
}

template <typename Callback, class Proxy, typename Func>
hook_info prepare_hook(vfunc<Func> const target)
{
    return prepare_hook<Callback, Proxy, Func>(target, std::true_type{});
}

template <typename Callback, FD_HOOK_PROXY_TEMPLATE class Proxy = detail::hook_proxy, typename Func>
hook_info prepare_hook(vfunc<Func> const target)
{
    return prepare_hook<Callback, Proxy<Func>, Func>(target, std::true_type{});
}

template <class Backend>
bool create_hook(Backend* backend, hook_info const& info)
{
    auto original      = backend->create(info.target, info.replace);
    auto const created = static_cast<bool>(original);
    if (created)
        global_hook_original_func = original;
    return created;
}

template <FD_HOOK_PROXY_TEMPLATE class Proxy, class Callback, class Backend, class Target>
bool create_hook(Backend* backend, Target const target)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info);
}

template <class Proxy, class Backend, class Callback, class Target>
bool create_hook(Backend* backend, Target const target)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info);
}

template <class Backend, class Callback, class Target>
bool create_hook(Backend* backend, Target const target)
{
    auto info = prepare_hook<Callback>(target);
    return create_hook(backend, info);
}
} // namespace fd