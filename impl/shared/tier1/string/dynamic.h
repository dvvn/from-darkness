#pragma once
#include "tier0/core.h"

#include <string>

namespace FD_TIER(1)
{
template <typename Chr>
using basic_string = std::basic_string<Chr>;

using string   = std::string;
using u8string = std::u8string;
using wstring  = std::wstring;
} // namespace FD_TIER(1)
