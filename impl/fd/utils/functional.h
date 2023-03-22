#pragma once

#include <optional>

namespace fd
{
template <
    typename Fn,
    bool Trivial = std::equality_comparable_with<Fn, nullptr_t> && /**/ std::assignable_from<Fn, nullptr_t>>
class invoke_on_destruct;

template <typename Fn>
class invoke_on_destruct<Fn, true>
{
    Fn fn_;

  public:
    ~invoke_on_destruct()
    {
        if (fn_ != nullptr)
            fn_();
    }

    invoke_on_destruct(Fn fn)
        : fn_(std::move(fn))
    {
    }

    invoke_on_destruct(invoke_on_destruct const &other) = delete;

    invoke_on_destruct(invoke_on_destruct &&other) noexcept
        : fn_(std::exchange(other.fn_, nullptr))
    {
    }

    invoke_on_destruct &operator=(invoke_on_destruct const &other) = delete;

    invoke_on_destruct &operator=(invoke_on_destruct &&other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }
};

template <typename Fn>
class invoke_on_destruct<Fn, false>
{
    std::optional<Fn> fn_;

  public:
    ~invoke_on_destruct()
    {
        if (fn_.has_value())
            (*fn_)();
    }

    invoke_on_destruct(Fn fn)
        : fn_(std::move(fn))
    {
    }

    invoke_on_destruct(invoke_on_destruct const &other) = delete;

    invoke_on_destruct(invoke_on_destruct &&other) noexcept
        : fn_(std::exchange(other.fn_, std::nullopt))
    {
    }

    invoke_on_destruct &operator=(invoke_on_destruct const &other) = delete;

    invoke_on_destruct &operator=(invoke_on_destruct &&other) noexcept
    {
        using std::swap;
        swap(fn_, other.fn_);
        return *this;
    }
};

template <typename Fn>
invoke_on_destruct(Fn fn) -> invoke_on_destruct<std::decay_t<Fn>>;

inline void *vfunc(void *instance, size_t idx)
{
    return (*static_cast<void ***>(instance))[idx];
}

template <typename Ret, typename... Args>
Ret vfunc(void *instance, size_t idx, Args... args)
{
    using fn_t = Ret(__thiscall *)(void *, Args...);
    return (*static_cast<fn_t **>(instance))[idx](instance, static_cast<Args>(args)...);
}

template <typename Ret, typename... Args>
Ret vfunc(void const *instance_const, size_t idx, Args... args)
{
    auto instance = const_cast<void *>(instance_const);
    using fn_t    = Ret(__thiscall *)(void *, Args...);
    return (*static_cast<fn_t **>(instance))[idx](instance, static_cast<Args>(args)...);
}

} // namespace fd