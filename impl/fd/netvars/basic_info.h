#pragma once

#include <string_view>

namespace fd
{
struct basic_netvar_info
{
    virtual ~basic_netvar_info() = default;

    virtual size_t           offset() const = 0;
    virtual std::string_view name() const   = 0;
    virtual std::string_view type() const   = 0;

    virtual basic_netvar_info* clone() const = 0;
};
}