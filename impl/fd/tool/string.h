#pragma once

#include "core.h"

#include <boost/static_string.hpp>

#include <string>

namespace fd
{
using std::basic_string;

FD_WRAP_TOOL(string, std::string);
FD_WRAP_TOOL(u8string, std::u8string);
FD_WRAP_TOOL(wstring, std::wstring);

using boost::static_strings::basic_static_string;

template <size_t Length>
FD_WRAP_TOOL(static_string, boost::static_strings::static_string<Length>);
template <size_t Length>
FD_WRAP_TOOL(static_wstring, boost::static_strings::static_wstring<Length>);
template <size_t Length>
FD_WRAP_TOOL(static_u8string, boost::static_strings::static_u8string<Length>);
} // namespace fd