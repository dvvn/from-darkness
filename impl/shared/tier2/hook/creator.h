#pragma once

#include "tier2/hook/info.h"
#include "tier2/hook/proxy.h"

namespace FD_TIER(2)
{
template <class Backend, class Callback>
bool create_hook(Backend& backend, hook_info<Callback> const& info, Callback* callback)
{
    auto original      = backend.create(info.target(), info.replace());
    auto const created = static_cast<bool>(original);
    if (created)
    {
        auto& data    = detail::unique_hook_proxy_data<Callback>;
        data.original = original;
        data.callback = callback;
    }
    return created;
}

template <FD_HOOK_PROXY_TEMPLATE class Proxy, class Backend, class Target, class Callback>
bool create_hook(Backend& backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Proxy, class Backend, class Target, class Callback>
bool create_hook(Backend& backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Backend, class Target, class Callback>
bool create_hook(Backend& backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback>(target);
    return create_hook(backend, info, callback);
}
} // namespace FD_TIER(2)