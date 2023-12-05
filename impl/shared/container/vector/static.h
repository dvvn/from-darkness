#pragma once



// ReSharper disable once CppUnusedIncludeDirective
#include "external/boost_vector.h"

#include <boost/container/static_vector.hpp>

namespace fd
{
template <typename T, size_t BufferSize>
using static_vector = boost::container::static_vector<T, BufferSize>;
}