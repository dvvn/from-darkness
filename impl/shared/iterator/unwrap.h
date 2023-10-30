#pragma once

#include <boost/move/detail/iterator_to_raw_pointer.hpp>

namespace boost::container
{
template <class Pointer, bool IsConst>
class vec_iterator;
}

// ReSharper disable CppInconsistentNaming

_STD_BEGIN
template <typename T, bool C>
_NODISCARD T _Get_unwrapped(boost::container::vec_iterator<T, C> it) noexcept
{
    return it.get_ptr();
}

_STD_END

namespace boost::movelib
{
#if defined(_MSC_VER) && _ITERATOR_DEBUG_LEVEL != 0
namespace detail
{
template <class Iterator>
requires(_STD _Unwrappable_v<Iterator>)
auto iterator_to_pointer(Iterator const& i) -> typename iterator_to_element_ptr<Iterator>::type
{
    return iterator_to_pointer(_STD _Get_unwrapped(i));
}
} // namespace detail

template <class Iterator>
requires(_STD _Unwrappable_v<Iterator>)
auto iterator_to_raw_pointer(Iterator const& i) -> typename detail::iterator_to_element_ptr<Iterator>::type
{
    return iterator_to_raw_pointer(_STD _Get_unwrapped(i));
}
#endif
} // namespace boost::movelib

// ReSharper restore CppInconsistentNaming

namespace fd::detail
{
/// deprecated
// using boost::movelib::iterator_to_raw_pointer;
// using boost::movelib::to_raw_pointer;

template <typename It>
decltype(auto) unwrap_iterator(It&& it)
{
#ifdef _MSC_VER
    return _STD _Get_unwrapped(it);
#else
#endif
}

template <typename It, typename ItRaw>
void rewrap_iterator(It& it, ItRaw&& it_raw)
{
#ifdef _MSC_VER
    return _STD _Seek_wrapped(it, it_raw);
#else
#endif
}

template <bool Validate = true, typename It>
auto unwrap_range(It first, It const last)
{
    if constexpr (Validate)
    {
#ifdef _MSC_VER
        _STD _Adl_verify_range(first, last);
#else
#endif
    }
    return std::pair(unwrap_iterator(first), unwrap_iterator(last));
}

template <bool Validate = true, typename Rng>
auto unwrap_range(Rng&& rng)
{
#ifdef _DEBUG
    static_assert(std::is_trivially_destructible_v<Rng> || std::is_lvalue_reference_v<Rng>);
#endif
    using std::begin;
    using std::end;
    return unwrap_range<Validate>(begin(rng), end(rng));
}

} // namespace fd::detail
