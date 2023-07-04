#pragma once

#include "internal/wrapper.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "iterator/unwrap.h"

#include <boost/container/small_vector.hpp>

namespace fd
{
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(small_vector, boost::container::small_vector<T, BufferSize>);
}