#pragma once

#include <fmt/core.h>

FMT_BEGIN_NAMESPACE

template <typename Fn>
class lazy_format_value
{
    Fn fn_;

  public:
    FMT_CONSTEXPR lazy_format_value(Fn fn)
        : fn_(std::move(fn))
    {
    }

    FMT_CONSTEXPR decltype(auto) operator()() const
    {
        return fn_();
    }

    FMT_CONSTEXPR decltype(auto) operator()()
    {
        return fn_();
    }
};

template <typename Fn>
lazy_format_value(Fn) -> lazy_format_value<std::decay_t<Fn>>;

template <typename Fn>
FMT_CONSTEXPR lazy(Fn &&fn)
{
    return lazy_format_value(std::forward<Fn>(fn));
}

template <typename Fn, typename C>
struct formatter<lazy_format_value<Fn>, C> : formatter<std::decay_t<std::invoke_result_t<Fn>>, C>
{
    using base = formatter<std::invoke_result_t<Fn>, C>;

    auto format(lazy_format_value<Fn> const &f, auto &ctx) const -> decltype(ctx.out())
    {
        return base::format(f(), ctx);
    }

    auto format(lazy_format_value<Fn> &f, auto &ctx) const -> decltype(ctx.out())
    {
        return base::format(f(), ctx);
    }
};

FMT_END_NAMESPACE