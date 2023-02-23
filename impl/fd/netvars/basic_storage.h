#pragma once

#include <string_view>

namespace fd
{
struct basic_netvars_storage
{
    virtual ~basic_netvars_storage()                                                   = default;
    virtual size_t get_offset(std::string_view className, std::string_view name) const = 0;
};

basic_netvars_storage* get_netvars_storage();
} // namespace  fd