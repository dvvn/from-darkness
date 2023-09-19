#pragma once

#include "basic_backend.h"
// ReSharper disable CppUnusedIncludeDirective
#include "object_holder.h"
#include "proxy.h"
// ReSharper restore CppUnusedIncludeDirective

#define FD_HOOK_FWD(_T_, _IFC_, ...)                                             \
    template <>                                                                  \
    struct object_info<_T_> final : incomplete_object_info<_IFC_, ##__VA_ARGS__> \
    {                                                                            \
        using hook_data = prepared_hook_data_abstract<wrapped>;                  \
        static hook_data construct(args_packed packed_args);                     \
    };
#define FD_HOOK_IMPL(_T_, ...)                                                      \
    auto object_info<_T_, false>::construct(args_packed packed_args) -> hook_data   \
    {                                                                               \
        using std::move;                                                            \
        auto callback             = object_info<_T_, true>::construct(packed_args); \
        unique_hook_callback<_T_> = static_cast<_T_*>(callback);                    \
        auto target               = callback->target();                             \
        auto hook_data            = prepare_hook<_T_, ##__VA_ARGS__>(move(target)); \
        return {move(hook_data), move(callback)};                                   \
    }

namespace fd
{
template <typename CallbackWrapped>
struct prepared_hook_data_abstract : prepared_hook_data
{
    CallbackWrapped callback;
};
} // namespace fd