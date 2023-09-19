#pragma once

#include <boost/move/detail/iterator_to_raw_pointer.hpp>

// ReSharper disable CppInconsistentNaming
#if defined(_MSC_VER) && _ITERATOR_DEBUG_LEVEL != 0
namespace boost
{
namespace container
{
template <class Pointer, bool IsConst>
class vec_iterator;
}

namespace movelib
{
namespace detail
{
template <class Iterator>
    requires(std::_Unwrappable_v<Iterator>)
typename iterator_to_element_ptr<Iterator>::type iterator_to_pointer(Iterator const& i)
{
    return iterator_to_pointer(std::_Get_unwrapped(i));
}
} // namespace detail

template <class Iterator>
    requires(std::_Unwrappable_v<Iterator>)
typename detail::iterator_to_element_ptr<Iterator>::type iterator_to_raw_pointer(Iterator const& i)
{
    return iterator_to_raw_pointer(std::_Get_unwrapped(i));
}
} // namespace movelib
} // namespace boost

_STD_BEGIN
template <typename T, bool C>
_NODISCARD T _Get_unwrapped(boost::container::vec_iterator<T, C> it) noexcept
{
    return it.get_ptr();
}

_STD_END
#endif
// ReSharper restore CppInconsistentNaming

namespace fd
{
using boost::movelib::iterator_to_raw_pointer;
using boost::movelib::to_raw_pointer;
} // namespace fd
