#pragma once

#include <boost/preprocessor/cat.hpp>

#define FD_RANDOM_NAME BOOST_PP_CAT(__random, __COUNTER__)
#define FD_UNUSED_VAR  auto const FD_RANDOM_NAME

#define FD_GROUP_ARGS(...) __VA_ARGS__

#ifdef _DEBUG
#define FD_DEBUG_RELEASE(_D_, _R_) _D_
#else
#define FD_DEBUG_RELEASE(_D_, _R_) _R_
#endif

// #define FD_CONSTEXPR_OPT FD_DEBUG_RELEASE(const, constexpr)

namespace fd
{

} // namespace fd
