#pragma once

#include "object.h"
#include "gui/basic_menu_item.h"

#include <cstdint>

namespace fd
{
struct string_view;
struct wstring_view;

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
    virtual basic_netvar_table const *inner(string_view name) const = 0;
    virtual basic_netvar_info const *get(string_view name) const    = 0;
};

struct native_client_class;
struct native_data_map;

struct basic_netvar_storage : basic_object, basic_menu_item
{
    virtual basic_netvar_table const *get(string_view name) const = 0;

    virtual void store(native_client_class const *root) = 0;
    virtual void store(native_data_map const *root)     = 0;

    virtual void save(wstring_view directory) const            = 0;
    virtual void load(wstring_view directory, uint8_t version) = 0;
};
}