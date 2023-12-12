#pragma once

#include "hook/info.h"
#include "hook/proxy.h"

namespace fd
{
template <class Backend, class Callback>
bool create_hook(Backend* const backend, hook_info<Callback> const& info, Callback* const callback)
{
    auto original      = backend->create(info.target(), info.replace());
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
bool create_hook(Backend* const backend, Target const target, Callback* const callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Proxy, class Backend, class Target, class Callback>
bool create_hook(Backend* const backend, Target const target, Callback* const callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Backend, class Target, class Callback>
bool create_hook(Backend* const backend, Target const target, Callback* const callback)
{
    auto info = prepare_hook<Callback>(target);
    return create_hook(backend, info, callback);
}

template <class Backend>
class create_hook_helper
{
    Backend* const backend_;

  public:
    create_hook_helper(Backend* const backend)
        : backend_{backend}
    {
    }

    template <class Target, class Callback>
    bool operator()(Target const target, Callback* const callback) const
    {
        auto info = prepare_hook<Callback>(target);
        return create_hook(backend_, info, callback);
    }
};

} // namespace fd