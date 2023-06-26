#pragma once

#include "core.h"

#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>

#include <string>
#include <string_view>

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

using std::basic_string_view;

FD_WRAP_TOOL(string_view, std::string_view);
FD_WRAP_TOOL(u8string_view, std::u8string_view);
FD_WRAP_TOOL(wstring_view, std::wstring_view);

template <size_t S>
constexpr size_t strlen(char const (&)[S])
{
    return S - 1;
}

constexpr bool islower(char c)
{
    return c >= 'a' && c <= 'z';
}

constexpr bool isupper(char c)
{
    return c >= 'A' && c <= 'Z';
}

constexpr bool isdigit(char c)
{
    return c >= '0' && c <= '9';
}
} // namespace fd