#pragma once

#include "mem_backup.h"
#include "vfunc.h"

namespace fd
{
template <class T>
inline constexpr call_type_t vtable_call_type = detail::default_call_type_member;

template <call_type Call_T>
class vfunc_index
{
    size_t index_;

  public:
    constexpr vfunc_index(size_t index /*, call_type_holder<Call_T> = {}*/)
        : index_(index)
    {
    }

    operator size_t() const
    {
        return index_;
    }
};

template <class T>
struct vtable;

template <class T>
struct vtable_expander;

template <class T>
vtable<T>* safe_cast(vtable_expander<T>* ptr)
{
    return safe_cast<vtable<T>>(ptr);
}

template <class T>
vtable<T> const* safe_cast(vtable_expander<T> const* ptr)
{
    return safe_cast<vtable<T>>(ptr);
}

template <class T>
struct vtable_expander
{
#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename... Args>                                       \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, safe_cast<T>(this)->instance()};                              \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <>
struct vtable_expander<void>
{
#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename T, typename... Args>                           \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, safe_cast<T>(this)->instance()};                              \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <class T>
struct vtable : vtable_expander<T>
{
    using instance_pointer = T*;
    using table_pointer    = void**;

  private:
    union
    {
        instance_pointer instance_;
        table_pointer* vtable_;
    };

  public:
    using vtable_expander<T>::operator[];

    vtable()
        : instance_(nullptr)
    {
    }

    /*explicit*/ vtable(void* instance) requires(!std::is_void_v<T>)
        : instance_(static_cast<instance_pointer>(instance))
    {
    }

    vtable(instance_pointer instance)
        : instance_(instance)
    {
    }

    instance_pointer operator->() const
    {
        return instance_;
    }

    instance_pointer instance() const
    {
        return instance_;
    }

    operator instance_pointer() const
    {
        return instance_;
    }

    /*instance_pointer operator->() const
    {
        return instance_;
    }*/

    /*operator table_pointer() const
    {
        return *vtable_;
    }*/

    table_pointer get() const
    {
        return *vtable_;
    }

    void set(table_pointer pointer)
    {
        *vtable_ = pointer;
    }

    mem_backup<table_pointer> replace(table_pointer pointer) &
    {
        return make_mem_backup(*vtable_, pointer);
    }

    template <typename Q>
    mem_backup<table_pointer> replace(vtable<Q> other) &
    {
        return make_mem_backup(*vtable_, other.get());
    }

    template <call_type Call_T>
    unknown_vfunc_args<Call_T, T> operator[](vfunc_index<Call_T> index) const
    {
        return {index, instance_};
    }

    unknown_vfunc_args<vtable_call_type<T>, T> operator[](ptrdiff_t index) const
    {
        return {index, instance_};
    }
};

template <typename T>
auto get(vtable<T> table, auto index) -> decltype(table[index])
{
    return table[index];
}

template <typename T>
vtable(T*, size_t = 0) -> vtable<T>;

// template<call_type_t Call_T,typename Ret,class T,typename ...Args>
// class vfunc_view
//{
//     vtable<T>
// };
}