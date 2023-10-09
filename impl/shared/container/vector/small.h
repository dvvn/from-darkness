#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include "iterator/unwrap.h"

#include <boost/container/small_vector.hpp>

namespace fd
{
template <typename T, size_t BufferSize>
using small_vector = boost::container::small_vector<T, BufferSize>;
}