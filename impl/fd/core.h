#pragma once

#include <utility>

namespace fd
{
#if 0
template <typename T>
using _const = std::add_const_t<T>;

template <typename T>
constexpr _const<T>* as_const(T *ptr)
{
    return ptr;
}

#else
template <typename T>
struct add_const_all : std::add_const<T>
{
};

template <typename T>
struct add_const_all<T &&>;

template <typename T>
struct add_const_all<T &> : std::add_const<T>
{
    using type = std::add_const_t<T> &;
};

template <typename T>
struct add_const_all<T *>
{
    using type = std::add_const_t<T> *;
};

template <typename T>
struct add_const_all<T **>
{
    using type = std::add_const_t<typename add_const_all<T *>::type> *;
};

template <typename T>
using _const = typename add_const_all<T>::type;

template <typename T>
constexpr _const<T *> as_const(T *ptr)
{
    return ptr;
}
#endif

// using std::as_const;

} // namespace fd