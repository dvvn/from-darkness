#pragma once

#include "interface.h"

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

struct native_client_class;
struct native_data_map;

struct basic_netvar_storage : basic_interface
{
    virtual basic_netvar_table *get(string_view name) = 0;
    virtual void store(native_client_class *root)     = 0;
    virtual void store(native_data_map *root)         = 0;
};
}