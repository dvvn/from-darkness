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
    auto interface(string_view name) const
    {
        auto found = interface(name);
        if constexpr (std::is_union_v<T>)
            return static_cast<T>(found);
        else
            return static_cast<T *>(found);
    }
};
} // namespace fd