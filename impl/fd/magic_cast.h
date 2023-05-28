#pragma once

#include "magic_cast_base.h"

namespace fd
{
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
    explicit(std::is_same_v<From, To>) magic_cast(From from) requires(magic_convertible<From, To>)
        : from_(from)
    {
    }

    [[deprecated]] //
    magic_cast(From from, To hint) requires(!std::is_same_v<From, To>)
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



template <typename From>
class magic_cast<From, auto_cast_tag>
{
    From from_;

  public:
    magic_cast(From from)
        : from_(from)
    {
    }

    template <typename T>
    magic_cast(magic_cast<T, From> val)
        : from_(val.to_)
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
    operator To() const requires(magic_convertible<From, To>)
    {
        return magic_cast<From, To>(from_);
    }

    template <typename To>
    To to() const
    {
        return magic_cast<From, To>(from_);
    }
};

template <typename From, typename To>
struct auto_cast_resolver
{
    consteval auto_cast_resolver()
    {
        static_assert(magic_convertible<From, To>);
    }

    To operator()(From from) const
    {
        return magic_cast<From, To>(from);
    }
};

template <typename From, typename To>
struct auto_cast_resolver<magic_cast<From, auto_cast_tag>, To>
{
    consteval auto_cast_resolver()
    {
        static_assert(magic_convertible<From, To>);
    }

    To operator()(magic_cast<From, auto_cast_tag> from) const
    {
        return from.template to<To>();
    }
};

template <typename To>
class magic_cast<auto_cast_tag, To> : public magic_cast_helper<auto_cast_tag, To>
{
    friend class magic_cast<To, auto_cast_tag>;
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
    magic_cast(From from, auto_cast_resolver<From, To> resolver = {})
        : to_(resolver(from))
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

template <typename From>
using from = magic_cast<From, auto_cast_tag>;

template <typename From>
magic_cast(From) -> magic_cast<From, auto_cast_tag>;

template <typename To>
using to = magic_cast<auto_cast_tag, To>;

// template <typename From>
// magic_cast(From from) -> magic_cast<std::decay_t<From>, auto_cast_tag>;
} // namespace fd