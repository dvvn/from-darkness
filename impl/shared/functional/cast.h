#pragma once

#include <utility>

namespace fd
{
using std::as_const;

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

template <class L, class R /*, size_t L_Size = sizeof(L), size_t R_Size = sizeof(R)*/>
constexpr void can_unsafe_cast()
{
    static_assert(sizeof(L) == sizeof(R));
}

template <typename To, typename From>
concept native_unsafe_cast = requires(From from) { reinterpret_cast<To>(from); };

template <typename To, typename From>
To unsafe_cast(From from)
{
    if constexpr (std::convertible_to<To, From>)
    {
        return static_cast<To>(from);
    }
    else
    {
        // static_assert(sizeof(To) == sizeof(From));
        can_unsafe_cast<To, From>();

        if constexpr (native_unsafe_cast<To, From>)
        {
            return reinterpret_cast<To>(from);
        }
        else
        {
            union
            {
                From from0;
                To to;
            };

            from0 = from;
            return to;
        }
    }
}
}