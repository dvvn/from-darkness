#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include ".detail/boost_iterator_unwrapper.h"
#include ".detail/wrapper.h"

#include <boost/container/small_vector.hpp>

namespace fd
{
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(small_vector, boost::container::small_vector<T, BufferSize>);
}