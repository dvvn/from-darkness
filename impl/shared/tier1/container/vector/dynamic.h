#pragma once

#include "tier0/core.h"

#include <vector>

namespace FD_TIER(1)
{
template <typename T>
using vector = std::vector<T>;

template <typename T, typename Alloc>
using vector_ex = std::vector<T, Alloc>;

template <typename T, template <typename> class Alloc>
using vector_ex2 = std::vector<T, Alloc<T>>;
}