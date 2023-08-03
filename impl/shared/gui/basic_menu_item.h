#pragma once
#include "named_object.h"

namespace fd
{
struct basic_menu_item;
struct string_view;

struct basic_joined_menu_items
{
    using iterator = basic_menu_item *const *;

  protected:
    ~basic_joined_menu_items() = default;

  public:
    virtual iterator begin() const = 0;
    virtual iterator end() const   = 0;
};

struct basic_menu_item : basic_named_object
{
  protected:
    ~basic_menu_item() = default;

  public:
    virtual void render() = 0;

    virtual basic_joined_menu_items *child_joined() const
    {
        return nullptr;
    }

    virtual basic_menu_item *child() const
    {
        return nullptr;
    }
};
} // namespace fd
