#pragma once

#include <string_view>

namespace fd
{
struct basic_netvar_type
{
    virtual ~basic_netvar_type()         = default;
    virtual std::string_view get() const = 0;
};


extern size_t netvar_type_array_size(basic_netvar_type *type);

}