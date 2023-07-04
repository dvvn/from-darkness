#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include ".detail/boost_iterator_unwrapper.h"

#include <boost/container/static_vector.hpp>

namespace fd
{
template <typename T, size_t BufferSize>
FD_WRAP_TOOL(static_vector, boost::container::static_vector<T, BufferSize>);
}