#pragma once

#include <boost/noncopyable.hpp>

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