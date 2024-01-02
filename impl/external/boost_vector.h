#pragma once

#include <boost/move/core.hpp>

#include <utility>

namespace boost::container
{
template <typename Pointer, bool IsConst>
class vec_iterator;
}

// ReSharper disable CppRedundantInlineSpecifier
// ReSharper disable CppInconsistentNaming

#ifdef _MSC_VER
_STD_BEGIN

template <typename Pointer, bool IsConst>
_NODISCARD Pointer _Get_unwrapped(boost::container::vec_iterator<Pointer, IsConst> it) BOOST_NOEXCEPT_OR_NOTHROW
{
    return it.get_ptr();
}

namespace ranges
{
template <class Sent, typename Pointer, bool IsConst>
_NODISCARD decltype(auto) _Unwrap_iter(boost::container::vec_iterator<Pointer, IsConst> it) BOOST_NOEXCEPT_OR_NOTHROW
{
    _STL_INTERNAL_STATIC_ASSERT(sentinel_for<remove_cvref_t<Sent>, boost::container::vec_iterator<Pointer, IsConst>>);
    return it.get_ptr();
}

template <class Iter, typename Pointer, bool IsConst>
_NODISCARD decltype(auto) _Unwrap_sent(boost::container::vec_iterator<Pointer, IsConst> const se) BOOST_NOEXCEPT_OR_NOTHROW
{
    _STL_INTERNAL_STATIC_ASSERT(sentinel_for<boost::container::vec_iterator<Pointer, IsConst>, remove_cvref_t<Iter>>);
    return se.get_ptr();
}
} // namespace ranges

_STD_END

namespace boost::movelib
{
template <class Iterator>
BOOST_MOVE_FORCEINLINE auto iterator_to_raw_pointer(Iterator const& i) -> decltype(_STD _Get_unwrapped(i))
    requires(std::is_pointer_v<decltype(_STD _Get_unwrapped(i))>)
{
    return _STD _Get_unwrapped(i);
}

#ifdef _DEBUG
template <typename Pointer, bool IsConst>
BOOST_MOVE_FORCEINLINE Pointer iterator_to_raw_pointer(container::vec_iterator<Pointer, IsConst> it)
{
    return it.get_ptr();
}
#endif
} // namespace boost::movelib
#endif