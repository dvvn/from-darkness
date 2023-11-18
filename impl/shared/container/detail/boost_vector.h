#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <boost/config.hpp>

namespace boost::container
{
template <class Pointer, bool IsConst>
class vec_iterator;
}

_STD_BEGIN

template <typename T, bool C>
// ReSharper disable once CppInconsistentNaming
_NODISCARD T _Get_unwrapped(boost::container::vec_iterator<T, C> it) BOOST_NOEXCEPT_OR_NOTHROW
{
    return it.get_ptr();
}

namespace ranges
{
template <class Sent, typename T, bool C>
// ReSharper disable once CppInconsistentNaming
_NODISCARD decltype(auto) _Unwrap_iter(boost::container::vec_iterator<T, C> it) BOOST_NOEXCEPT_OR_NOTHROW
{
    _STL_INTERNAL_STATIC_ASSERT(sentinel_for<remove_cvref_t<Sent>, boost::container::vec_iterator<T, C>>);
    return it.get_ptr();
}

template <class Iter, typename T, bool C>
// ReSharper disable once CppInconsistentNaming
_NODISCARD constexpr decltype(auto) _Unwrap_sent(boost::container::vec_iterator<T, C> const se) BOOST_NOEXCEPT_OR_NOTHROW
{
    _STL_INTERNAL_STATIC_ASSERT(sentinel_for<boost::container::vec_iterator<T, C>, remove_cvref_t<Iter>>);
    return se.get_ptr();
}
} // namespace ranges

_STD_END