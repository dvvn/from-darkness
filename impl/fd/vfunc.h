#pragma once

namespace fd
{
constexpr void *vfunc(void *instance, size_t idx)
{
    return (*static_cast<void ***>(instance))[idx];
}

template <typename Ret, typename... Args>
constexpr Ret vfunc(void *instance, size_t idx, Args... args)
{
    using fn_t = Ret(__thiscall *)(void *, Args...);
    return (*static_cast<fn_t **>(instance))[idx](instance, static_cast<Args>(args)...);
}

template <typename Ret, typename... Args>
constexpr Ret vfunc(void const *instance, size_t idx, Args... args)
{
    using fn_t = Ret(__thiscall *const)(void const *, Args...);
    return (*static_cast<fn_t **>(instance))[idx](instance, static_cast<Args>(args)...);
}
}