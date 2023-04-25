#pragma once

#include <type_traits>

namespace fd
{
template <typename From, typename To>
class magic_cast
{
    union
    {
        From from_;
        To to_;
    };

  public:
    magic_cast(From from)
        : from_(from)
    {
        static_assert(sizeof(From) == sizeof(To));
    }

    magic_cast(From from, To)
        : magic_cast(from)
    {
    }

    operator To() const
    {
        return to_;
    }

    /*decltype(auto) operator*() const
    {
        return *to_;
    }*/
};

template <typename From, typename To>
magic_cast(From, To) -> magic_cast<std::decay_t<From>, std::decay_t<To>>;

template <typename From>
using to_void = magic_cast<From, void *>;

template <typename To>
using from_void = magic_cast<void *, To>;
}