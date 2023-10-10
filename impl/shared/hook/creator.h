#pragma once

#include "proxy.h"

namespace fd
{
template <class Backend, class Target, class Callback>
prepared_hook_data create_hook(Backend* backend, Target target, Callback* callback)
{
    unique_hook_callback<Callback> = callback;
    auto hook_data                 = prepare_hook<Callback>(target);
    backend->create(&hook_data);
    return hook_data;
}
} // namespace fd