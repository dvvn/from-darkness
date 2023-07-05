#pragma once
#include "system.h"

namespace fd
{
struct native_library_info : system_library_info
{
    using system_library_info::system_library_info;

    void *interface(string_view name) const;
};

template <library_tag Tag>
struct native_library;
} // namespace fd