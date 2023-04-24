#pragma once

#include <type_traits>

namespace fd
{
using netvar_id = char const *;

template <typename T>
struct netvar_getter
{
    using value_type = T;

    using pointer = std::conditional_t<
        std::is_pointer_v<T>, //
        T,
        std::add_pointer_t<T>>;
    using const_pointer = std::conditional_t<
        std::is_pointer_v<T>, //
        std::add_pointer_t<std::add_const_t<std::remove_pointer_t<T>>>,
        std::add_pointer_t<std::add_const_t<T>>>;

    netvar_id id;
    size_t offset;

    pointer get(void *thisptr) const
    {
        return reinterpret_cast<pointer>(reinterpret_cast<uintptr_t>(thisptr)) + offset;
    }

    const_pointer get(void const *thisptr) const
    {
        return reinterpret_cast<const_pointer>(reinterpret_cast<uintptr_t>(thisptr)) + offset;
    }
};
} // namespace fd