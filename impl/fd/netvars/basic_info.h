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
};

bool operator<(basic_netvar_info const& l, basic_netvar_info const& r);
} // namespace fd