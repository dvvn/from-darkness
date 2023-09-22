#pragma once
#include "named_object.h"

namespace fd
{
struct basic_menu_item;
struct string_view;

struct basic_menu_item : basic_named_object
{
  protected:
    ~basic_menu_item() = default;

  public:
    virtual void render() const = 0;

    virtual basic_menu_item* child()
    {
        return nullptr;
    }
};
} // namespace fd
