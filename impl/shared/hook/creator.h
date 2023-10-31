#pragma once

#include "hook/info.h"
#include "hook/proxy.h"

namespace fd
{
template <class Backend, class Target, class Callback>
hook_info<Callback> create_hook(Backend* backend, Target target, Callback* callback)
{
    auto info     = prepare_hook<Callback>(target);
    auto original = backend->create(info.target(), info.replace());
    assert(original != nullptr);
    auto& data = detail::unique_hook_proxy_data<Callback>;
    data.set_original(original);
    data.set_callback(callback);
    return info;
}

template <class Backend, class Target, class Callback>
hook_info<Callback> create_hook(Backend* backend, Target target, Callback callback) requires(std::is_trivially_destructible_v<Callback>)
{
    auto stored_callback = new (&detail::unique_hook_callback<Callback>) Callback(std::move(callback));
    return create_hook(backend, target, stored_callback);
}
} // namespace fd