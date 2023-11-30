#pragma once
#include "tier0/core.h"

#include <concepts>
#include <utility>

namespace FD_TIER(1)
{
template <typename Fn>
class invoke_cast
{
    [[no_unique_address]] //
    Fn func_;

  public:
    template <typename FnFwd>
    constexpr invoke_cast(FnFwd&& func)
        : func_(std::forward<FnFwd>(func))
    {
    }

    template <typename T>
    constexpr invoke_cast(invoke_cast<T> const& func) = delete;

    template <typename Ret>
    constexpr operator Ret() &&
#ifdef _DEBUG
        requires std::invocable<Fn, std::type_identity<Ret>>
#endif
    {
        return func_(std::type_identity<Ret>());
    }

    template <typename Ret>
    constexpr bool operator==(Ret other) &&
#ifdef _DEBUG
        requires std::invocable<Fn, std::type_identity<Ret>>
#endif
    {
        return func_(std::type_identity<Ret>()) == other;
    }
};

template <typename Fn>
invoke_cast(Fn&&) -> invoke_cast<Fn>;
}