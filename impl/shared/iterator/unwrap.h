#pragma once

#include <boost/move/detail/iterator_to_raw_pointer.hpp>

namespace boost
{
namespace container
{
template <class Pointer, bool IsConst>
class vec_iterator;
}

namespace movelib::detail
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
} // namespace movelib::detail
} // namespace boost

namespace fd
{
using boost::movelib::iterator_to_raw_pointer;
using boost::movelib::to_raw_pointer;
} // namespace fd

// ReSharper disable CppInconsistentNaming
#ifdef _MSC_VER
_STD_BEGIN
template <typename T, bool C>
BOOST_FORCEINLINE T _Get_unwrapped(boost::container::vec_iterator<T, C> it)
{
    return it.get_ptr();
}

_STD_END
#else

#endif
// ReSharper restore CppInconsistentNaming