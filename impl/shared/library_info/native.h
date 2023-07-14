#pragma once
#include "system.h"

namespace fd
{
struct native_library_info : system_library_info
{
    using return_address_checker_t = bool(__thiscall *)(void *, char const *);

    using system_library_info::system_library_info;

    void *interface(string_view name) const;
    return_address_checker_t return_address_checker() const;
};

template <library_tag Tag>
struct native_library;
} // namespace fd