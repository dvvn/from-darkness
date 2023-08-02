#pragma once
#include "named_object.h"

#include <cstdint>

namespace fd
{
struct basic_menu_item;
struct string_view;

struct joined_menu_items
{
  protected:
    ~joined_menu_items() = default;

  public:
    virtual basic_menu_item *begin() const = 0;
    virtual basic_menu_item *end() const   = 0;
};

struct basic_menu_item : basic_named_object
{
  protected:
    ~basic_menu_item() = default;

  public:
    virtual void render()                    = 0;
    virtual joined_menu_items *child() const = 0;
};
} // namespace fd
