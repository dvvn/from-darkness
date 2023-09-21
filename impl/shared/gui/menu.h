#pragma once
#include "basic_menu.h"

namespace fd
{
using unload_handler = basic_function<void>;

template <class T>
struct make_incomplete_object;
class menu;

template <>
struct make_incomplete_object<menu> final
{
    basic_menu* operator()(unload_handler const* handler) const;
};
} // namespace fd