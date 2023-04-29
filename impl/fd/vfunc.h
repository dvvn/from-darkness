#pragma once

#include <concepts>

namespace fd
{
template <typename From, typename To>
class magic_cast;

template <typename T>
constexpr size_t vtable_offset = 0;

class vfunc_holder
{
    void *func_;

  public:
    vfunc_holder(void *vfunc)
        : func_(vfunc)
    {
    }

    template <typename T>
    vfunc_holder(T *instance, size_t index)
        : func_(static_cast<void ***>((void *)instance)[vtable_offset<T>][index])
    {
    }

    operator void *() const
    {
        return func_;
    }

    void *get() const
    {
        return func_;
    }
};

template <typename Ret, typename... Args>
class vfunc : public vfunc_holder
{
    using fn_t = Ret(__thiscall *)(void *, Args...);

    void *instance_;

  public:
    template <typename T>
    vfunc(T *instance, size_t index)
        : vfunc_holder(instance, index)
        , instance_((void *)(instance))
    {
    }

    template <typename... ArgsFwd>
    Ret operator()(ArgsFwd &&...args) const
    {
        return reinterpret_cast<fn_t>(get())(instance_, std::forward<ArgsFwd>(args)...);
    }
};

template <>
class vfunc<nullptr_t> : public vfunc_holder
{
  public:
    using vfunc_holder::vfunc_holder;
};

template <typename T>
class vtable
{
    using instance_pointer = T *;
    using table_pointer    = void **;

    instance_pointer instance_;

  public:
    vtable(instance_pointer instance = nullptr)
        : instance_(instance)
    {
        static_assert(!std::is_pointer_v<T>);
    }

    template <typename From, typename To>
    vtable(magic_cast<From, To> val)
        : vtable(static_cast<instance_pointer>(val))
    {
    }

    /*vtable &operator=(std::convertible_to<pointer> auto instance)
    {
        instance_ = instance;
        return *this;
    }*/

    operator instance_pointer() const
    {
        return instance_;
    }

    instance_pointer operator->() const
    {
        return instance_;
    }

    operator table_pointer() const
    {
        return static_cast<void ***>(instance_)[vtable_offset<T>];
    }

    table_pointer get() const
    {
        return static_cast<void ***>(instance_)[vtable_offset<T>];
    }

    vfunc<nullptr_t> func(size_t index) const
    {
        return {instance_, index};
    }

    template <typename Ret, typename... Args>
    vfunc<Ret, Args...> func(size_t index) const
    {
        return {instance_, index};
    }

    template <typename Ret = void, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return vfunc<Ret, Args...>(instance_, index)(
            static_cast<std::conditional_t<
                std::is_trivially_copyable_v<Args> ? std::is_pointer_v<Args> || sizeof(Args) <= sizeof(uintptr_t[2])
                                                   : std::is_reference_v<Args>,
                Args,
                std::add_lvalue_reference_t<Args>>>(args)...);
    }
};

template <typename From, typename To>
vtable(magic_cast<From, To *>) -> vtable<To>;

// template <typename T>
// class vtable<T *> : public vtable<T>
//{
//   public:
//     using vtable<T>::vtable;
//     using vtable<T>::operator=;
// };
//
// template <typename T>
// class vtable<T **>
//{
//   public:
//     vtable(...) = delete;
// };

// template <typename T>
// vtable(T instance) -> vtable<std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, void>>;

template <typename T>
vtable(vtable<T>) -> vtable<void **>; // deleted

template <typename T>
struct cast_helper;

template <typename From, typename To>
vtable(magic_cast<From, cast_helper<To *>>) -> vtable<To>;

template <typename From, typename To>
magic_cast(vtable<From>, To) -> magic_cast<From *, To>;

template <typename From, typename To>
magic_cast(vtable<From *>, To) -> magic_cast<From *, To>;

template <typename To, typename... Args>
magic_cast(vfunc<Args...>, To) -> magic_cast<void *, To>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd