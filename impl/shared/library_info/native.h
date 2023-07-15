#pragma once
#include "system.h"

#undef interface

namespace fd
{
struct native_library_info : system_library_info
{
    using system_library_info::system_library_info;

    void *interface(char const *name, size_t length) const;

    template <size_t Length>
    void *interface(char const (&name)[Length]) const
    {
        return interface(name, Length - 1);
    }

    void* return_address_checker() const;
};

template <library_tag Tag>
struct native_library;
} // namespace fd