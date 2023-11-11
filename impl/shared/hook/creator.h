#pragma once

#include "hook/info.h"
#include "hook/proxy.h"

namespace fd
{
template <class Backend, class Callback>
bool create_hook(Backend* backend, hook_info<Callback> const& info, Callback* callback)
{
    auto original = backend->create(info.target(), info.replace());
    if (!original)
        return false;
    auto& data = detail::unique_hook_proxy_data<Callback>;
    data.set_original(original);
    data.set_callback(callback);
    return true;
}

template <FD_HOOK_PROXY_TEMPLATE class Proxy, class Backend, class Target, class Callback>
bool create_hook(Backend* backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Proxy, class Backend, class Target, class Callback>
bool create_hook(Backend* backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback, Proxy>(target);
    return create_hook(backend, info, callback);
}

template <class Backend, class Target, class Callback>
bool create_hook(Backend* backend, Target target, Callback* callback)
{
    auto info = prepare_hook<Callback>(target);
    return create_hook(backend, info, callback);
}

/*namespace detail
{
template <typename T>
inline uint8_t unique_hook_callback[sizeof(T)];
}

template <class Backend, class Target, class Callback>
hook_info<Callback> create_hook(Backend* backend, Target target, Callback callback) requires(std::is_trivially_destructible_v<Callback>)
{
    auto stored_callback = new (&detail::unique_hook_callback<Callback>) Callback(std::move(callback));
    return create_hook(backend, target, stored_callback);
}*/
} // namespace fd