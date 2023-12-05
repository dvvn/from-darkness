#pragma once


#include <array>

namespace fd
{
template <typename T, size_t Length>
using array = std::array<T, Length>;
}