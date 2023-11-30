#pragma once
#include "tier1/functional/bind.h"
#include "tier1/string/charconv.h"

#include <boost/lambda2.hpp>

namespace boost::lambda2
{
template <int I>
requires(I < 0)
struct lambda2_arg<I>;

template <int I_1, int I_2>
constexpr bool operator==(lambda2_arg<I_1>, lambda2_arg<I_2>)
{
    return I_1 == I_2;
}
} // namespace boost::lambda2

namespace FD_TIER(1)
{
namespace detail
{
template <char... C>
struct placeholder_literal
{
    static constexpr auto index = from_chars<int, 10, C...>();
    static constexpr boost::lambda2::lambda2_arg<index> value;
};
} // namespace detail

namespace placeholders
{
inline namespace literals
{
template <char... C>
constexpr auto operator"" _p()
{
    return boost::lambda2::lambda2_arg<from_chars<int, 10, C...>()>();
}
} // namespace literals

using namespace boost::lambda2;
} // namespace placeholders
}