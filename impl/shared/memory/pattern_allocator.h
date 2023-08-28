#pragma once

#include "basic_pattern.h"

#include <memory>

namespace fd::detail
{
template <typename T>
struct dynamic_pattern_allocator : std::allocator<T>
{
    using size_type       = pattern_size_type;
    using difference_type = pattern_difference_type;
};
} // namespace fd::detail
