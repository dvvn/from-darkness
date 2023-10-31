#pragma once

#include "hook/prepared_data.h"
#include "hook/proxy.h"

namespace fd
{
template <class Backend, class Target, class Callback>
prepared_hook_data<Callback> create_hook(Backend* backend, Target target, Callback* callback)
{
    auto hook_data = prepare_hook<Callback>(target);
    auto original  = backend->create(hook_data.target(), hook_data.replace());
    assert(original != nullptr);
    detail::unique_hook_proxy_data<Callback>.set_original(original);
    detail::unique_hook_proxy_data<Callback>.set_callback(callback);
    return hook_data;
}



template <class Backend, class Target, class Callback>
prepared_hook_data<Callback> create_hook(Backend* backend, Target target, Callback callback) requires(std::is_trivially_destructible_v<Callback>)
{
    auto stored_callback = new (&detail::unique_hook_callback<Callback>) Callback(std::move(callback));
    return create_hook(backend, target, stored_callback);
}
} // namespace fd