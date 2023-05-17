#pragma once

#include <concepts>

namespace fd
{
template <typename From, typename To>
class magic_cast;

template <typename From, typename To>
magic_cast<From, To> &operator+=(magic_cast<From, To> &obj, size_t offset) requires requires(To val) { val += offset; }
{
    To val = obj;
    val += offset;
    obj = val;
    return obj;
}

template <typename From, typename To>
magic_cast<From, To> &operator-=(magic_cast<From, To> &obj, size_t offset) requires requires(To val) { val -= offset; }
{
    To val = obj;
    val -= offset;
    obj = val;
    return obj;
}

template <typename From, typename To>
class magic_cast_helper_ex;

template <typename From, typename To>
class magic_cast_helper : public magic_cast_helper_ex<From, To>
{
    using base = magic_cast<From, To> const *;

  public:
    operator To() const
    {
        return static_cast<base>(this)->to_;
    }
};

#if 0
template <typename From, typename To>
class magic_cast_helper_arithmetic
{
    using base       = magic_cast<From, To> *;
    using const_base = magic_cast<From, To> const *;

  public:
    base &operator+=(size_t offset)
    {
        static_cast<base>(this)->to_ += offset;
        return *static_cast<base>(this);
    }

    base &operator-=(size_t offset)
    {
        static_cast<base>(this)->to_ -= offset;
        return *static_cast<base>(this);
    }

    base &operator*=(size_t offset)
    {
        static_cast<base>(this)->to_ *= offset;
        return *static_cast<base>(this);
    }

    base &operator/=(size_t offset)
    {
        static_cast<base>(this)->to_ /= offset;
        return *static_cast<base>(this);
    }

    /*base operator+(size_t offset) const
    {
        auto clone = *static_cast<base>(this);
        clone.to_ += offset;
        return clone;
    }

    base operator-(size_t offset) const
    {
        auto clone = *static_cast<base>(this);
        clone.to_ -= offset;
        return clone;
    }

    base operator*(size_t offset) const
    {
        auto clone = *static_cast<base>(this);
        clone.to_ *= offset;
        return clone;
    }

    base operator/(size_t offset) const
    {
        auto clone = *static_cast<base>(this);
        clone.to_ /= offset;
        return clone;
    }*/
};
#endif

template <typename From, typename To>
class magic_cast_helper_ex
{
    using base       = magic_cast<From, To> *;
    using const_base = magic_cast<From, To> const *;

  public:
    To const *operator->() const
    {
        return &static_cast<const_base>(this)->to_;
    }

    To *operator->()
    {
        return &static_cast<base>(this)->to_;
    }
};

template <typename From, typename To>
class magic_cast_helper_ex<From, To *> /*: public magic_cast_helper_arithmetic<From, To *>*/
{
    using base = magic_cast<From, To *> const *;

  public:
    To *operator->() const
    {
        return static_cast<base>(this)->to_;
    }

    To &operator[](ptrdiff_t i) const
    {
        return static_cast<base>(this)->to_[i];
    }
};

template <typename From>
class magic_cast_helper_ex<From, void *>
{
};

// WORKAROUND
template <typename From, typename Ret, typename... Args>
class magic_cast_helper_ex<From, Ret(__thiscall *)(Args...)>
{
};

template <typename From, typename To>
constexpr bool magic_convertible_v = sizeof(From) == sizeof(To) && std::is_trivial_v<From> && std::is_trivial_v<To>;

template <typename From, typename To>
class magic_cast : public magic_cast_helper<From, To>
{
    template <typename, typename>
    friend class magic_cast_helper;
    template <typename, typename>
    friend class magic_cast_helper_ex;

    union
    {
        From from_;
        To to_;
    };

  public:
    explicit(std::same_as<From, To>) magic_cast(From from) requires(magic_convertible_v<From, To>)
        : from_(from)
    {
    }

    magic_cast(From from, To hint) requires(!std::same_as<From, To>)
        : magic_cast(from)
    {
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
    operator To() const requires(magic_convertible_v<From, To>)
    {
        return magic_cast<From, To>(from_);
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
class magic_cast<auto_cast_tag, To> : public magic_cast_helper<auto_cast_tag, To>
{
    template <typename, typename>
    friend class magic_cast_helper;
    template <typename, typename>
    friend class magic_cast_helper_ex;

    To to_;

  public:
    magic_cast() = default;

    magic_cast(To to)
        : to_(to)
    {
    }

    template <typename From>
    magic_cast(From from)
        : to_(magic_cast<From, To>(from))
    {
    }

    template <std::derived_from<vfunc_holder> From>
    magic_cast(From from)
        : to_(magic_cast<void *, To>(from.get()))
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

    /*To get() const
    {
        return to_;
    }*/

    /*operator To() const
    {
        return to_;
    }

    To operator->() const
    {
        return to_;
    }*/
};

template <typename To>
[[deprecated]] //
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

template <typename From>
magic_cast(From) -> magic_cast<From, auto_cast_tag>;

template <typename To>
using to = magic_cast<auto_cast_tag, To>;

// template <typename From>
// magic_cast(From from) -> magic_cast<std::decay_t<From>, auto_cast_tag>;
} // namespace fd