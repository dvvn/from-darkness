#pragma once

#include <fd/string.h>

namespace fd
{
    struct basic_netvars_storage
    {
        virtual ~basic_netvars_storage() = default;

        virtual size_t get_offset(string_view className, string_view name) const = 0;
    };

    extern basic_netvars_storage* Netvars;
} // namespace  fd