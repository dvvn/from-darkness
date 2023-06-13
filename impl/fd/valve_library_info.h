#pragma once

#include "library_info.h"

#undef interface

namespace fd
{
struct valve_library : system_library
{
    using system_library::system_library;

    void *interface(string_view name) const;

    template <class T>
    T interface(string_view name) const
    {
        auto found = interface(name);
        if constexpr (std::is_pointer_v<T>)
            return static_cast<T>(found);
        else
            return found;
    }
};
} // namespace fd