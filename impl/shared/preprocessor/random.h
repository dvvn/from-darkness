#pragma once

#include <boost/preprocessor/cat.hpp>

#define FD_RANDOM_NAME BOOST_PP_CAT(__random, __COUNTER__)
#define FD_RANDOM_VAR auto const FD_RANDOM_NAME