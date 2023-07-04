#pragma once

#include <boost/core/ignore_unused.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/move/detail/iterator_to_raw_pointer.hpp>

#include <utility>

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


// using std::as_const;





using boost::ignore_unused;
using boost::noncopyable;

using boost::movelib::iterator_to_raw_pointer;
using boost::movelib::to_raw_pointer;

} // namespace fd