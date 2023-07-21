#pragma once

#include "basic_group.h"
#include "object.h"
#include "container/array.h"

#include <concepts>

namespace fd
{
struct variables_group : basic_variables_group, basic_object
{
};

template <std::convertible_to<basic_variables_group *>... T>
array<basic_variables_group *, sizeof...(T)> join(T &&...group)
{
    return {static_cast<basic_variables_group *>(group)...};
}
} // namespace fd