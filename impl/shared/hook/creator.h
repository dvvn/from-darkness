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
} // namespace fd