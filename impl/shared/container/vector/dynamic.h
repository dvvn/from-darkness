#pragma once

#include "internal/wrapper.h"

#include <vector>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(vector, std::vector<T>);

template <typename T, typename Alloc>
FD_WRAP_TOOL(vector_ex, std::vector<T, Alloc>);

template <typename T, template <typename> class Alloc>
FD_WRAP_TOOL(vector_ex2, std::vector<T, Alloc<T>>);
}