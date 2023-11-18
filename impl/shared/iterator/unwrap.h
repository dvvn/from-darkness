#pragma once

#include <iterator>
#include <utility>

// ReSharper restore CppInconsistentNaming

namespace fd
{
namespace detail
{
/// deprecated
// using boost::movelib::iterator_to_raw_pointer;
// using boost::movelib::to_raw_pointer;

template <typename Rng>
concept can_access_unwrapped_iterator = std::is_trivially_destructible_v<std::remove_cvref_t<Rng>> || std::is_lvalue_reference_v<Rng>;

template <typename T>
concept has_unchecked_begin = requires(T obj) {
#ifdef _MSC_VER
    static_cast<T>(obj)._Unchecked_begin();
#else

#endif
};

template <typename T>
concept has_unchecked_end = requires(T obj) {
#ifdef _MSC_VER
    static_cast<T>(obj)._Unchecked_end();
#else

#endif
};

} // namespace detail

template <typename It>
constexpr decltype(auto) unwrap_iterator(It&& it)
{
#ifdef _MSC_VER
    return _STD _Get_unwrapped(static_cast<It&&>(it));
#else
#endif
}

template <typename It, typename ItRaw>
constexpr void rewrap_iterator(It& it, ItRaw&& it_raw)
{
#ifdef _MSC_VER
    return _STD _Seek_wrapped(it, static_cast<ItRaw&&>(it_raw));
#else
#endif
}

template <typename Rng>
constexpr auto ubegin(Rng&& rng)
#ifdef _DEBUG
    requires(detail::can_access_unwrapped_iterator<Rng &&>)
#endif
{
    if constexpr (detail::has_unchecked_begin<Rng&&>)
    {
#ifdef _MSC_VER
        return static_cast<Rng&&>(rng)._Unchecked_begin();
#else

#endif
    }
    else
    {
        using std::begin;
        return unwrap_iterator(begin(static_cast<Rng&&>(rng)));
    }
}

template <typename It, typename It2>
constexpr void verify_range(It const& it, It2 const& it2)
{
#ifdef _MSC_VER
    _STD _Adl_verify_range(it, it2);
#else

#endif
}

template <typename Rng>
constexpr auto uend(Rng&& rng)
#ifdef _DEBUG
    requires(detail::can_access_unwrapped_iterator<Rng &&>)
#endif
{
    if constexpr (detail::has_unchecked_end<Rng&&>)
    {
#ifdef _MSC_VER
        return static_cast<Rng&&>(rng)._Unchecked_end();
#else

#endif
    }
    else
    {
        using std::end;
        return unwrap_iterator(end(static_cast<Rng&&>(rng)));
    }
}

} // namespace fd
