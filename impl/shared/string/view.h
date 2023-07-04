#pragma once

#include ".detail/wrapper.h"

#include <string_view>

namespace fd
{
using std::basic_string_view;

FD_WRAP_TOOL(string_view, std::string_view);
FD_WRAP_TOOL(u8string_view, std::u8string_view);
FD_WRAP_TOOL(wstring_view, std::wstring_view);
} // namespace fd