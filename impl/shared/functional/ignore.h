#pragma once
// ReSharper disable once CppUnusedIncludeDirective
#include <boost/core/ignore_unused.hpp>

namespace fd
{
#ifdef __RESHARPER__
constexpr void ignore_unused(auto &&...)
{
}
#else
using boost::ignore_unused;
#endif
}