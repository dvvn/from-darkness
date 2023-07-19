﻿#pragma once

#include "basic_group.h"
#include "interface.h"
#include "container/array.h"

#include <concepts>

namespace fd
{
struct variables_group : basic_variables_group, basic_interface
{
};

template <std::convertible_to<basic_variables_group *>... T>
array<basic_variables_group *, sizeof...(T)> join(T &&...group)
{
    return {static_cast<basic_variables_group *>(group)...};
}

template <size_t S>
constexpr uint8_t size(array<basic_variables_group *, S> const &)
{
    return S;
}
} // namespace fd