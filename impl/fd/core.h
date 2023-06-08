#pragma once

#include <boost/noncopyable.hpp>

#include <utility>

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

using boost::noncopyable;

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

} // namespace fd