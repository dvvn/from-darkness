#pragma once

#include <concepts>

namespace fd
{
template <typename From, typename To>
class magic_cast
{
    template <typename From1, typename To1>
    friend class magic_cast;

    union
    {
        From from_;
        To to_;
    };

  protected:
    From from() const
    {
        return from_;
    }

    To to() const
    {
        return to_;
    }

  public:
    /*template <class From1, class To1>
    magic_cast(magic_cast<From1, To1> other) requires(sizeof(From1) == sizeof(From))
    {
        if constexpr (std::assignable_from<From, From1>)
            from_ = other.from_;
        else
            from_ = static_cast<From>(other);
    }*/

    magic_cast(/*std::same_as<From> auto*/ From from)
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

template <typename From>
using to_void = magic_cast<From, void *>;

template <typename To>
using from_void = magic_cast<void *, To>;

// template <typename To>
// magic_cast<uintptr_t, To> operator+(magic_cast<uintptr_t, To> val, std::integral auto offset)
//{
//     return static_cast<uintptr_t>(val) + offset;
// }
//
// template <typename To>
// magic_cast<uintptr_t, To> &operator+=(magic_cast<uintptr_t, To> &val, std::integral auto offset)
//{
//     return val = static_cast<uintptr_t>(val) + offset;
// }

template <class>
struct magic_cast_traits;

template <typename From, typename To>
struct magic_cast_traits<magic_cast<From, To>>
{
    using from = From;
    using to   = To;

    template <typename T>
    using rebind_from = magic_cast<T, To>;

    template <typename T>
    using rebind_to = magic_cast<From, T>;
};

// template <typename From, typename To>
// magic_cast(From, To) -> magic_cast<std::decay_t<From>, std::decay_t<To>>;

struct auto_cast_tag final
{
};

template <typename From>
class magic_cast<From, auto_cast_tag>
{
  public:
    magic_cast(...) = delete;
};

template <>
class magic_cast<void *, auto_cast_tag> : public magic_cast<void *, uintptr_t>
{
    using magic_cast<void *, uintptr_t>::from;

  public:
    using magic_cast<void *, uintptr_t>::magic_cast;

    template <typename T>
    operator T &() const
    {
        return *static_cast<T *>(from());
    }

    template <typename... T>
    operator magic_cast<T...>() const = delete;

    template <typename T>
    operator T *() const
    {
        return static_cast<T *>(from());
    }
};

template <>
class magic_cast<uintptr_t, auto_cast_tag> : public magic_cast<uintptr_t, void *>
{
    using magic_cast<uintptr_t, void *>::to;

  public:
    using magic_cast<uintptr_t, void *>::magic_cast;

    template <typename T>
    operator T &() const
    {
        return *static_cast<T *>(to());
    }

    template <typename T>
    operator T *() const
    {
        return static_cast<T *>(to());
    }

    /*template <typename Ret, typename... Args>
    operator std::add_pointer_t<Ret(Args...)>() const
    {
        return static_cast<Ret (*)(Args...)>(from());
    }*/
};

// template <, typename To>
// class magic_cast<auto_cast_tag, To>
//{
//   public:
//     magic_cast(...) = delete;
// };

template <typename To>
class magic_cast<auto_cast_tag, To>
{
    To val_;

  public:
    template <typename From>
    magic_cast(From from)
        : val_(magic_cast<From, To>(from))
    {
    }

    operator To() const
    {
        return val_;
    }
};

template <typename To>
using from_any = magic_cast<auto_cast_tag, To>;

// template <typename From>
// magic_cast(From from) -> magic_cast<std::decay_t<From>, auto_cast_tag>;
} // namespace fd