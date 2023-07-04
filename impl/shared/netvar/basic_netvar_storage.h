#pragma once

#include <cstddef>

namespace fd
{
struct string_view;

class basic_netvar_info
{
  protected:
    ~basic_netvar_info() = default;

  public:
    virtual size_t offset() const = 0;
};

class basic_netvar_table
{
  protected:
    ~basic_netvar_table() = default;

  public:
    virtual basic_netvar_table *inner(string_view name) = 0;
    virtual basic_netvar_info *get(string_view name)    = 0;
};

class basic_netvar_storage
{
  protected:
    ~basic_netvar_storage() = default;

  public:
    virtual basic_netvar_table *get(string_view name) = 0;
};
}