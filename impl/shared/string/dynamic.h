﻿#pragma once

#include <string>

namespace fd
{
template <typename Chr>
using basic_string = std::basic_string<Chr>;

using string   = std::string;
using u8string = std::u8string;
using wstring  = std::wstring;
} // namespace fd
