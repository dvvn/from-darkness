#pragma once

#include "type_traits/type_identity.h"

namespace fd
{
template <bool Test, typename True, typename False>
struct conditional;

template <bool Test, typename True, typename False>
using conditional_t = typename conditional<Test, True, False>::type;

template <typename True, typename False>
struct conditional<true, True, False> : type_identity<True>
{
};

template <typename True, typename False>
struct conditional<false, True, False> : type_identity<False>
{
};
} // namespace fd