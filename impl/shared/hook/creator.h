#pragma once

#include "hook/prepared_data.h"
#include "hook/proxy.h"

namespace fd
{
template <class Backend, class Target, class Callback>
prepared_hook_data create_hook(Backend* backend, Target target, Callback* callback)
{
    auto hook_data = prepare_hook<Callback>(target);
    auto original  = backend->create(hook_data.target, hook_data.replace);
    assert(original != nullptr);
    *hook_data.original            = original;
    unique_hook_callback<Callback> = callback;
    return hook_data;
}

namespace detail
{
template <typename T>
inline uint8_t hook_callback_buffer[sizeof(T)];
}

template <class Backend, class Target, class Callback>
prepared_hook_data create_hook(Backend* backend, Target target, Callback callback) requires(std::is_trivially_destructible_v<Callback>)
{
    auto stored_callback = new (&detail::hook_callback_buffer<Callback>) Callback(std::move(callback));
    return create_hook(backend, target, stored_callback);
}
} // namespace fd