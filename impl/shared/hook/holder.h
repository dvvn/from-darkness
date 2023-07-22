#pragma once

#include "basic_backend.h"
#include "callback.h"
#include "object_holder.h"

#define FD_HOOK_FWD(_T_, _IFC_, ...)                                             \
    template <>                                                                  \
    struct object_info<_T_> final : incomplete_object_info<_IFC_, ##__VA_ARGS__> \
    {                                                                            \
        using wrapped = prepared_hook_data_abstract<wrapped>;                    \
        static wrapped construct(void *target, args_packed args_packed);         \
    };
#define FD_HOOK_IMPL(_T_, ...)                                        \
    auto object_info<_T_, false>::construct(                          \
        void *target, /**/                                            \
        args_packed packed_args)                                      \
        ->wrapped                                                     \
    {                                                                 \
        using info_t  = object_info<_T_, true>;                       \
        auto callback = info_t::construct(packed_args);               \
        init_hook_callback(static_cast<info_t::unwrapped>(callback)); \
        auto hook_data = prepare_hook<_T_, ##__VA_ARGS__>(target);    \
        return {std::move(hook_data), std::move(callback)};           \
    }

namespace fd
{
template <typename CallbackWrapped>
struct prepared_hook_data_abstract : prepared_hook_data
{
    CallbackWrapped callback;
};

template <typename Callback, typename... Args>
auto prepare_hook(void *target, Args &&...args) -> typename object_info<Callback>::wrapped
{
    return make_object<Callback>(target, args...);
}

template <typename Callback, typename... Args>
auto prepare_hook(auto &&target, Args &&...args) -> typename object_info<Callback>::wrapped
    requires requires { target.get(); }
{
    return prepare_hook<Callback>(target.get(), args...);
}

}