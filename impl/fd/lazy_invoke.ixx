module;

#include <functional>
#include <optional>

export module fd.lazy_invoke;

namespace std
{
    template <typename Fn>
    decltype(auto) invoke(const optional<Fn>& fn)
    {
        return invoke(*fn);
    }

    template <typename Fn>
    decltype(auto) invoke(optional<Fn>& fn)
    {
        return invoke(*fn);
    }
} // namespace std

template <typename Fn>
concept func_maybe_null = requires(const Fn fn)
{
    !fn;
};

template <typename Fn>
struct lazy_invoke
{
    using value_type = std::conditional_t<func_maybe_null<Fn>, Fn, std::optional<Fn>>;
    using func_type  = Fn;

  private:
    value_type fn_;

  public:
    constexpr ~lazy_invoke()
    {
        if (!fn_)
            return;
        std::invoke(fn_);
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
    }
};

template <typename Fn>
lazy_invoke(const Fn&) -> lazy_invoke<Fn>;

export namespace fd
{
    using ::lazy_invoke;
}
