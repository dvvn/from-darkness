#pragma once
#include "named_object.h"

namespace fd
{
struct basic_menu_item : basic_named_object
{
  protected:
    ~basic_menu_item() = default;

  public:
    virtual void render() const = 0;
};
} // namespace fd
