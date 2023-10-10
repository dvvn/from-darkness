#pragma once
#ifndef __RESHARPER__
#include <boost/core/ignore_unused.hpp>
#endif

namespace fd
{
#ifdef __RESHARPER__
constexpr void ignore_unused(auto&&...)
{
}
#else
using boost::ignore_unused;
#endif
}