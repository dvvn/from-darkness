#pragma once

#include "x86_call.h"

#include <concepts>

namespace fd
{
template <typename From, typename To>
class magic_cast;

template <typename T>
[[deprecated("store this value inside class")]] //
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

template <typename T>
class vfunc_instance_holder : public vfunc_holder
{
    T *instance_;

  public:
    vfunc_instance_holder(T *instance, size_t index)
        : vfunc_holder(instance, index)
        , instance_((instance))
    {
    }

    T *instance() const
    {
        return instance_;
    }
};

size_t get_vfunc_index(void *instance, size_t vtable_offset, void *function, _x86_call call);

template <typename Fn>
void *member_function_pointer(Fn function)
{
    union
    {
        Fn fn;
        void *fn_ptr;
    };

    fn = function;
    return fn_ptr;
}

#define GET_VFUNC_INDEX(call__, __call)                                                              \
    template <typename Ret, typename T, typename... Args>                                            \
    size_t get_vfunc_index(void *instance, size_t vtable_offset, Ret (__call T::*function)(Args...)) \
    {                                                                                                \
        return get_vfunc_index(instance, vtable_offset, member_function_pointer(function), call__);  \
    }

X86_CALL_MEMBER(GET_VFUNC_INDEX);
#undef GET_VFUNC_INDEX

template <_x86_call Call, typename Ret, typename T, typename... Args>
struct vfunc_x86;

template <typename Ret, typename T, typename... Args>
struct vfunc_x86<_x86_call::unknown, Ret, T, Args...>
{
    vfunc_x86(...) = delete;
};

template <typename T>
struct vfunc_x86<_x86_call::unknown, void, T> : vfunc_holder
{
    using vfunc_holder::vfunc_holder;
};

template <typename T>
vfunc_x86(T *, size_t) -> vfunc_x86<_x86_call::unknown, void, T>;

#define VFUNC_X86(call__, __call)                                                                 \
    template <typename Ret, typename T, typename... Args>                                         \
    struct vfunc_x86<call__, Ret, T, Args...> : vfunc_instance_holder<T>                          \
    {                                                                                             \
        using vfunc_instance_holder<T>::instance;                                                 \
        using vfunc_instance_holder<T>::get;                                                      \
        using vfunc_instance_holder<T>::vfunc_instance_holder;                                    \
        vfunc_x86(T *instance, Ret (__call T::*fn)(Args...))                                      \
            : vfunc_instance_holder<T>(instance, get_vfunc_index(instance, vtable_offset<T>, fn)) \
        {                                                                                         \
        }                                                                                         \
        Ret operator()(Args... args) const                                                        \
        {                                                                                         \
            return x86_invoker<call__, Ret>(instance(), get(), static_cast<Args>(args)...);       \
        }                                                                                         \
    };                                                                                            \
    template <typename Ret, typename T, typename... Args>                                         \
    vfunc_x86(T *, Ret (__call T::*)(Args...)) -> vfunc_x86<call__, Ret, T, Args...>;

X86_CALL_MEMBER(VFUNC_X86);
#undef VFUNC_X86

template </*std::convertible_to<nullptr_t>*/ typename P, typename Fn>
auto vfunc(P *instance, Fn fn)
{
    return vfunc_x86(instance, fn);
}

template <typename T>
struct vtable
{
    using instance_pointer = T *;
    using table_pointer    = void **;

  private:
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

    void set(table_pointer pointer) const
    {
        static_cast<void ***>(instance_)[vtable_offset<T>] = pointer;
    }

    auto func(size_t index) const
    {
        return vfunc(instance_, index);
    }

    template <typename Fn>
    auto func(Fn fn) const requires(std::is_member_function_pointer_v<Fn>)
    {
        return vfunc(instance_, fn);
    }

    template <typename Ret, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return x86_invoker<_x86_call::thiscall__, Ret>(instance_, func(index), static_cast<Args>(args)...);
    }

    template <_x86_call Call, typename Ret, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return x86_invoker<Call, Ret>(instance_, func(index), static_cast<Args>(args)...);
    }

    /*template <typename Ret = void, typename... Args>
    Ret call(size_t index, Args... args) const
    {
        return vfunc<Ret, Args...>(instance_, index)(
            static_cast<std::conditional_t<
                std::is_trivially_copyable_v<Args> ? std::is_pointer_v<Args> || sizeof(Args) <= sizeof(uintptr_t[2])
                                                   : std::is_reference_v<Args>,
                Args,
                std::add_lvalue_reference_t<Args>>>(args)...);
    }*/
};

// template <typename T>
// auto exchange(vtable<T> table, typename vtable<T>::table_pointer ptr) -> decltype(ptr)
//{
//     auto backup = table.get();
//     table.set(ptr);
//     return backup;
// }

template <typename T, typename T2 = void>
auto exchange(vtable<T> table, vtable<T2> other) -> typename vtable<T>::table_pointer
{
    auto backup = table.get();
    table.set(other.get());
    return backup;
}

template <typename From, typename To>
vtable(magic_cast<From, To *>) -> vtable<To>;

struct auto_cast_tag;

template <typename From>
vtable(magic_cast<From, auto_cast_tag>) -> vtable<void>;

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

// template <typename From, typename To>
// magic_cast(vtable<From *>, To) -> magic_cast<From *, To>;

template <_x86_call Call, typename Ret, typename T, typename To, typename... Args>
magic_cast(vfunc_x86<Call, Ret, T, Args...>, To) -> magic_cast<T *, To>;

template <typename To>
magic_cast(vfunc_holder, To) -> magic_cast<void *, To>;

// template <typename T>
// vtable(T const *instance) -> vtable<T const *>;

} // namespace fd

namespace std
{
template <typename T>
void **exchange(fd::vtable<T> vt, void **new_table)
{
    void **old_table = vt;
}
}