#pragma once
#include "basic_menu.h"
#include "object_holder.h"

namespace fd
{
template <typename Ret, typename... Args>
class basic_function;

using unload_handler = basic_function<void>;

class menu;
FD_OBJECT_FWD(menu, basic_menu, unload_handler const *);
} // namespace fd