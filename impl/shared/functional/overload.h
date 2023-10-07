#pragma once

#include <boost/hana/functional.hpp>

#include <functional>

template <typename T>
requires(std::is_member_pointer_v<T>)
struct boost::hana::overload_t<T>
{
    using type = decltype(std::bind_front(std::declval<T>()));
};

template <typename T>
requires(std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>)
struct boost::hana::overload_t<T>
{
    using type = decltype(std::bind_front(std::declval<T>()));
};

// bind_front return reference, but we dont need reference to pointer in 90% cases
template <typename T, class Obj>
struct boost::hana::overload_t<T * Obj::*>
{
  private:
    using stored_t = T* Obj::*;

    stored_t stored_;

  public:
    using type = overload_t;

    constexpr explicit overload_t(stored_t stored)
        : stored_(stored)
    {
    }

    T* operator()(Obj const* obj) const
    {
#ifdef __cpp_lib_invoke_r
        return std::invoke_r<T*>(stored_, obj);
#else
        return std::invoke(stored_, obj);
#endif
    }
};

namespace fd
{
using boost::hana::overload;
template <typename Fn>
using overload_t = typename boost::hana::overload_t<std::decay_t<Fn>>::type;
}