#pragma once

#include <boost/core/ignore_unused.hpp>
#include <boost/core/noncopyable.hpp>

#include <utility>

//todo: split this file in multiple 

namespace fd
{
using boost::ignore_unused;
using boost::noncopyable;

using std::as_const;
using std::unreachable;

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

namespace detail
{
template <size_t A, size_t B>
constexpr void validate_size()
{
    static_assert(A == B);
}
} // namespace detail

template <typename Fn>
Fn void_to_func(void *function)
{
    if constexpr (std::convertible_to<void *, Fn>)
        return static_cast<Fn>(function);
    else
    {
        // static_assert(sizeof(Fn) == sizeof(void *));
        detail::validate_size<sizeof(Fn), sizeof(void *)>();

        union
        {
            void *from;
            Fn to;
        };

        from = function;
        return to;
    }
}

} // namespace fd