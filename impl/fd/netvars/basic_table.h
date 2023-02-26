#pragma once

#include <fd/netvars/basic_info.h>

#include <functional>

namespace fd
{
struct basic_netvar_table
{
    using for_each_fn = std::function<void(basic_netvar_info const&)>;

    virtual ~basic_netvar_table() = default;

    virtual std::string_view name() const = 0;

    virtual bool empty() const
    {
        return size() != 0;
    }

    virtual size_t size() const = 0;

    virtual void for_each(for_each_fn const& fn) const = 0;
};

} // namespace fd