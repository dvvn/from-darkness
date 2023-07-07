#pragma once

#include "hook/basic_backend.h"

namespace fd
{
struct backend_minhook;

template <interface_type T>
constexpr bool is_valid_interface_v<T, backend_minhook> = true;

template <>
struct interface_creator<interface_type::heap, backend_minhook>
{
    using holder = unique_interface<basic_hook_backend>;
    static holder get();
};

template <>
struct interface_creator<interface_type::stack, backend_minhook>
{
    using pointer = basic_hook_backend *;
    static pointer get();
};

} // namespace fd