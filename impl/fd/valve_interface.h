#pragma once

#include "vtable.h"

namespace fd
{
struct string_view;

class valve_interface
{
    vtable<void> ptr_;

  public:
    valve_interface(string_view name);
    //valve_interface(string_view name, system_string_view library);
};
}