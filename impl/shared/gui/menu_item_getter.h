#pragma once
#include "basic_menu_item.h"
#include "functional/basic_function.h"

namespace fd
{
using menu_item_getter = basic_function<basic_menu_item*, size_t&>;
}