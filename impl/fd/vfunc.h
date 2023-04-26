#pragma once

#include <type_traits>

namespace fd
{
template <typename Ret, typename... Args>
class vfunc
{
    void *instance_;
    size_t vtable_offset_;
    size_t index_;

    using fn_t = Ret(__thiscall *)(void *, Args...);

  public:
    vfunc(void *instance, size_t vtable_offset, size_t index)
        : instance_(instance)
        , vtable_offset_(vtable_offset)
        , index_(index)
    {
    }

    vfunc(void *instance, size_t index)
        : instance_(instance)
        , vtable_offset_(0)
        , index_(index)
    {
    }

    template <typename... ArgsFwd>
    Ret operator()(ArgsFwd &&...args) const
    {
        return static_cast<fn_t **>(instance_)[vtable_offset_][index_](instance_, std::forward<Args>(args)...);
    }

    operator void *() const
    {
        return static_cast<void ***>(instance_)[vtable_offset_][index_];
    }
};

#ifndef _DEBUG
template <>
class vfunc<nullptr_t>
{
    void *func_;

  public:
    vfunc(void *vfunc)
        : func_(vfunc)
    {
    }

    vfunc(void *instance, size_t vtable_offset, size_t index)
        : func_(static_cast<void ***>(instance)[vtable_offset][index])
    {
    }

    operator void *() const
    {
        return func_;
    }
};
#endif

template <typename T>
class vtable
{
    T *instance_;
    size_t offset_;

  public:
    vtable(T *instance, size_t offset = 0)
        : instance_(instance)
        , offset_(offset)
    {
    }

    void **get() const
    {
        return static_cast<void ***>(instance_)[offset_];
    }

    auto func(size_t index) const -> vfunc<nullptr_t>
    {
        return {instance_, offset_, index};
    }

    template <typename Ret, typename... Args>
    auto func(size_t index) const -> vfunc<Ret, Args...>
    {
        return {instance_, offset_, index};
    }

    template <typename Ret = void, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        auto fn = vfunc<Ret, Args...>(instance_, offset_, index);
        return fn(
            static_cast<std::conditional_t<
                std::is_trivially_copyable_v<Args> ? sizeof(Args) <= sizeof(uintptr_t[2]) : std::is_reference_v<Args>,
                Args,
                std::add_lvalue_reference_t<Args>>>(args)...);
    }
};

template <typename T>
struct vtable<T const> : vtable<T>
{
    vtable(T const *instance, size_t offset = 0)
        : vtable<T>(const_cast<T *>(instance), offset)
    {
    }
};

template <typename T>
vtable(T instance) -> vtable<std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, void>>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd