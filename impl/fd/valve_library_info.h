#pragma once

#include "library_info.h"

#undef interface

namespace fd
{
struct valve_library : system_library
{
    using system_library::system_library;

    void *interface(string_view name) const;
};
} // namespace fd