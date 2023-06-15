#pragma once

#include <cstddef>

namespace fd
{
struct string_view;

struct basic_netvar_info
{
    virtual size_t offset() const = 0;
};

struct basic_netvar_table
{
    virtual basic_netvar_table *inner(string_view name) = 0;
    virtual basic_netvar_info *get(string_view name)    = 0;
};

struct basic_netvar_storage
{
    virtual basic_netvar_table *get(string_view name) = 0;
};
}