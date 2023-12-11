#pragma once

#include <string_view>

namespace fd
{
template <typename Chr>
using basic_string_view = std::basic_string_view<Chr>;

using string_view   = std::string_view;
using u8string_view = std::u8string_view;
using wstring_view  = std::wstring_view;
} // namespace fd