#pragma once

#include "basic_backend.h"
#include "callback.h"
#include "object_holder.h"

#define FD_HOOK_FWD(_T_, _IFC_, ...)                                             \
    template <>                                                                  \
    struct object_info<_T_> final : incomplete_object_info<_IFC_, ##__VA_ARGS__> \
    {                                                                            \
        using hook_data = prepared_hook_data_abstract<wrapped>;                  \
        static hook_data construct(void *target, args_packed args_packed);       \
    };
#define FD_HOOK_IMPL(_T_, ...)                                                \
    auto object_info<_T_, false>::construct(                                  \
        void *target, /**/                                                    \
        args_packed packed_args)                                              \
        ->hook_data                                                           \
    {                                                                         \
        using info_t              = object_info<_T_, true>;                   \
        auto hook_callback        = info_t::construct(packed_args);           \
        unique_hook_callback<_T_> = hook_callback;                            \
        auto hook_data            = prepare_hook<_T_, ##__VA_ARGS__>(target); \
        return {std::move(hook_data), std::move(hook_callback)};              \
    }

namespace fd
{
template <typename CallbackWrapped>
struct prepared_hook_data_abstract : prepared_hook_data
{
    CallbackWrapped callback;
};

template <typename Callback, typename... Args>
auto prepare_hook(void *target, Args &&...args)
{
    return make_object<Callback>(target, args...);
}

template <typename Callback, typename... Args>
auto prepare_hook(auto &&target, Args &&...args) requires requires { target.get(); }
{
    return prepare_hook<Callback>(target.get(), args...);
}

}