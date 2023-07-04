#pragma once

#include ".detail/wrapper.h"

#include <list>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(list, std::list<T>);
}