#pragma once

#include <concepts>

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
    explicit(std::same_as<From, To>) magic_cast(/*std::same_as<From> auto*/ From from)
        : from_(from)
    {
        static_assert(sizeof(From) == sizeof(To));
        static_assert(std::is_trivial_v<From>);
        static_assert(std::is_trivial_v<To>);
    }

    magic_cast(From from, To hint) requires(!std::same_as<From, To>)
        : magic_cast(from)
    {
    }

    operator To() const
    {
        return to_;
    }

    To operator->() const
    {
        return to_;
    }
};

template <typename From, typename To>
struct magic_cast<From, To &> : magic_cast<From, To *>
{
    using magic_cast<From, To *>::magic_cast;

    operator To &() const
    {
        return *static_cast<To *>(*this);
    }
};

struct auto_cast_tag final
{
};

template <typename From>
class magic_cast<From, auto_cast_tag>
{
    From from_;

  public:
    magic_cast(From from)
        : from_(from)
    {
    }

    // it works not how i expect
    /*template <typename To>
    operator To &() const
    {
    }*/

    operator From() const
    {
        return from_;
    }

    /*template <typename... T>
    operator magic_cast<T...>() const = delete;*/

    template <typename To>
    operator To *() const
    {
        return magic_cast<From, To *>(from_);
    }

    template <typename To>
    To to() const
    {
        return magic_cast<From, To>(from_);
    }
};

template <typename T>
struct cast_helper;

// template <typename Ret, typename... T>
// class vfunc;

class vfunc_holder;

template <typename To>
class magic_cast<auto_cast_tag, To>
{
    To to_;

  public:
    template <typename From>
    magic_cast(From from)
        : to_(magic_cast<From, To>(from))
    {
    }

    template <std::derived_from<vfunc_holder> From>
    magic_cast(From from)
        : to_(magic_cast<void *, To>(from))
    {
    }

    template <typename From>
    magic_cast(magic_cast<From, auto_cast_tag> from)
        : to_(from.template to<To>())
    {
    }

    /*template <typename... Args>
    magic_cast(vfunc<Args...> from)
        : magic_cast(from.get())
    {
    }*/

    operator To() const
    {
        return to_;
    }

    To operator->() const
    {
        return to_;
    }
};

template <typename To>
class magic_cast<auto_cast_tag, cast_helper<To>> : public magic_cast<auto_cast_tag, To>
{
    [[no_unique_address]] //
    cast_helper<To> /**/ helper_;

  public:
    template <typename From>
    magic_cast(From from)
        : magic_cast<auto_cast_tag, To>(helper_(from))
    {
    }
};

template <typename From>
using from = magic_cast<From, auto_cast_tag>;

template <typename To>
using to = magic_cast<auto_cast_tag, To>;

// template <typename From>
// magic_cast(From from) -> magic_cast<std::decay_t<From>, auto_cast_tag>;
} // namespace fd