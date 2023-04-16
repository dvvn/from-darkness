#pragma once

#include <fmt/core.h>

FMT_BEGIN_NAMESPACE

template <typename Fn>
class lazy_format_value
{
    Fn fn_;

  public:
    constexpr lazy_format_value(Fn fn)
        : fn_(std::move(fn))
    {
    }

    constexpr auto get() const
    {
        return fn_();
    }

    constexpr auto get()
    {
        return fn_();
    }
};

template <typename Fn>
lazy_format_value(Fn) -> lazy_format_value<std::decay_t<Fn>>;

template <typename Fn>
auto lazy(Fn &&fn)
{
    return lazy_format_value(std::forward<Fn>(fn));
}

template <typename Fn, typename C>
struct formatter<lazy_format_value<Fn>, C> : formatter<std::decay_t<std::invoke_result_t<Fn>>, C>
{
    using base = formatter<std::invoke_result_t<Fn>, C>;

    auto format(lazy_format_value<Fn> const &f, auto &ctx) const -> decltype(ctx.out())
    {
        return base::format(f.get(), ctx);
    }

    auto format(lazy_format_value<Fn> &f, auto &ctx) const -> decltype(ctx.out())
    {
        return base::format(f.get(), ctx);
    }
};

FMT_END_NAMESPACE