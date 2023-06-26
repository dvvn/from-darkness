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
template <typename T>
constexpr T const *as_const(T *ptr)
{
    return ptr;
}

template <typename T>
constexpr T const *as_const(T const *ptr)
{
    return ptr;
}

// using std::as_const;

template <typename T>
T *remove_const(T const *ptr)
{
    return const_cast<T *>(ptr);
}

template <typename T>
constexpr T *remove_const(T *ptr)
{
    return ptr;
}

template <typename T>
T &remove_const(T const &ref)
{
    return const_cast<T &>(ref);
}

template <typename T>
constexpr T &remove_const(T &ref)
{
    return (ref);
}

template <typename Fn>
void *get_function_pointer(Fn function)
{
    static_assert(sizeof(Fn) == sizeof(void *));

    union
    {
        Fn fn;
        void *ptr;
    };

    fn = function;
    return ptr;
}

using boost::ignore_unused;
using boost::noncopyable;

using boost::movelib::iterator_to_raw_pointer;
using boost::movelib::to_raw_pointer;

} // namespace fd