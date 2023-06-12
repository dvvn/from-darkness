#pragma once
#include "core.h"

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

#include <vector>

namespace fd
{
template <typename T>
FD_WRAP_TOOL(vector, std::vector<T>);
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(small_vector, boost::container::small_vector<T, BufferSize>);
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(static_vector, boost::container::static_vector<T, BufferSize>);
}