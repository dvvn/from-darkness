#pragma once

#include "tier0/core.h"

// ReSharper disable once CppUnusedIncludeDirective
#include "external/boost_vector.h"

#include <boost/container/static_vector.hpp>

namespace FD_TIER(1)
{
template <typename T, size_t BufferSize>
using static_vector = boost::container::static_vector<T, BufferSize>;
}