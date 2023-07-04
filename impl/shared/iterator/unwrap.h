#pragma once

#include <boost/move/detail/iterator_to_raw_pointer.hpp>

namespace boost::movelib::detail
{
#ifdef _MSC_VER
template <class It>
requires(std::_Unwrappable_v<It>)
// ReSharper disable once CppRedundantInlineSpecifier
BOOST_MOVE_FORCEINLINE auto iterator_to_pointer(It const &it)
{
    return iterator_to_pointer(std::_Get_unwrapped(it));
}
#else

#endif
} // namespace boost::movelib::detail

namespace fd
{
using boost::movelib::iterator_to_raw_pointer;
using boost::movelib::to_raw_pointer;
}