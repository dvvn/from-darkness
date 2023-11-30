#pragma once
#include "tier0/core.h"

#include <string_view>

namespace FD_TIER(1)
{
template <typename Chr>
using basic_string_view = std::basic_string_view<Chr>;

using string_view   = std::string_view;
using u8string_view = std::u8string_view;
using wstring_view  = std::wstring_view;

namespace string_view_literals = std::string_view_literals;
} // namespace FD_TIER(1)