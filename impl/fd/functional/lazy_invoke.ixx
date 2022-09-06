module;

#include <concepts>
#include <memory>

export module fd.functional.lazy_invoke;
import fd.functional.invoke;

template <typename T>
constexpr void _Reset(T& obj)
{
    if constexpr (std::is_class_v<T> || std::is_union_v<T>)
        std::destroy_at(&obj);
    else
        obj = nullptr;
}

template <typename Fn, bool = std::is_class_v<Fn> || std::is_union_v<Fn>>
struct correct_func;

template <typename Fn>
struct correct_func<Fn, false>
{
    using type = std::add_pointer_t<Fn>;
};

template <typename Fn>
struct correct_func<Fn, true>
{
    using type = Fn;
};

using fd::invoke;

template <typename Fn, bool = std::is_convertible_v<Fn, bool> && (!std::is_class_v<Fn> || !std::is_union_v<Fn> || std::assignable_from<Fn, nullptr_t>)>
class lazy_invoke;

template <typename Fn>
class lazy_invoke<Fn, true>
{
    typename correct_func<Fn>::type fn_;

  public:
    constexpr ~lazy_invoke()
    {
        if (fn_)
            invoke(fn_);
    }

    template <typename Fn1>
    constexpr lazy_invoke(Fn1&& fn)
        : fn_(std::forward<Fn1>(fn))
    {
    }

    constexpr lazy_invoke(lazy_invoke&& other) noexcept
    {
        *this = std::move(other);
    }

    constexpr lazy_invoke& operator=(lazy_invoke&& other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }

    constexpr void reset()
    {
        _Reset(fn_);
    }
};

template <typename Fn>
class lazy_invoke<Fn, false>
{
    union
    {
        typename correct_func<Fn>::type fn_;
        uint8_t gap_;
    };

    bool valid_;

  public:
    constexpr ~lazy_invoke()
    {
        if (valid_)
            invoke(fn_);
    }

    template <typename Fn1>
    constexpr lazy_invoke(Fn1&& fn)
        : fn_(std::forward<Fn1>(fn))
        , valid_(true)
    {
    }

    constexpr lazy_invoke(lazy_invoke&& other) noexcept
    {
        *this = std::move(other);
    }

    constexpr lazy_invoke& operator=(lazy_invoke&& other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        swap(valid_, other.valid_);
        return *this;
    }

    constexpr void reset()
    {
        _Reset(fn_);
        valid_ = false;
    }
};

#if 0
template <typename Fn>
struct lazy_invoke
{
    using value_type0 = std::conditional_t<func_maybe_null<Fn>, Fn, std::optional<Fn>>;
    using value_type  = std::conditional_t<std::is_class_v<value_type0>, value_type0, std::add_pointer_t<value_type0>>;
    using func_type   = Fn;

  private:
    value_type fn_;

  public:
    constexpr ~lazy_invoke()
    {
        if (!fn_)
            return;
        fd::invoke(fn_);
    }

    template <typename Fn1>
    constexpr lazy_invoke(Fn1&& fn)
        : fn_(std::forward<Fn1>(fn))
    {
    }

    constexpr lazy_invoke(lazy_invoke&& other) noexcept
    {
        *this = std::move(other);
    }

    constexpr lazy_invoke& operator=(lazy_invoke&& other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }

    constexpr void reset()
    {
        if constexpr (func_maybe_null<Fn>)
            fn_ = nullptr;
        else
            fn_.reset();
    }
};
#endif

template <typename Fn>
lazy_invoke(const Fn&) -> lazy_invoke<Fn>;

export namespace fd
{
    using ::lazy_invoke;
}
