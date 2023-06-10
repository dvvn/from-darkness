#pragma once

#include "core.h"

#include <string>

namespace fd
{
using std::basic_string;

FD_WRAP_TOOL(string, std::string);
FD_WRAP_TOOL(u8string, std::u8string);
FD_WRAP_TOOL(wstring, std::wstring);
} // namespace fd
