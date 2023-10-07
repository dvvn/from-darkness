#pragma once

#include "basic_backend.h"
#include "proxy.h"

namespace fd
{
#if 0
template <typename CallbackWrapped>
struct prepared_hook_data_full : prepared_hook_data
{
    CallbackWrapped callback;

    template <typename T>
    prepared_hook_data_full(prepared_hook_data const& data, T&& callback)
        : prepared_hook_data(data)
        , callback(std::forward<T>(callback))
    {
    }

    template <std::convertible_to<CallbackWrapped> T>
    prepared_hook_data_full(prepared_hook_data_full<T>&& other)
        : prepared_hook_data(static_cast<prepared_hook_data&&>(other))
        , callback(std::move(other.callback))
    {
    }
};

template <class Callback, typename... Args>
auto prepare_hook_wrapped(Args&&... args) -> prepared_hook_data_full<Callback*>
{
    auto callback                  = make_object<Callback>(std::forward<Args>(args)...);
    unique_hook_callback<Callback> = static_cast<Callback*>(callback);
    auto target                    = callback->target();
    return {prepare_hook<Callback>(target), std::move(callback)};
}
#endif
} // namespace fd