#pragma once
#include "wrapper.h"


#define BOOST_STATIC_STRING_STANDALONE
#include <boost/static_string.hpp>

namespace fd
{
using boost::static_strings::basic_static_string;

template <size_t Length>
FD_WRAP_TOOL(static_string, boost::static_strings::static_string<Length>);
template <size_t Length>
FD_WRAP_TOOL(static_wstring, boost::static_strings::static_wstring<Length>);
template <size_t Length>
FD_WRAP_TOOL(static_u8string, boost::static_strings::static_u8string<Length>);
} // namespace fd
