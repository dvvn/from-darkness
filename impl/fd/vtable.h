#pragma once

#include "mem_backup.h"
#include "vfunc.h"

namespace fd
{
template <typename T>
struct basic_vtable
{
    template <typename>
    friend struct vtable;

    using instance_pointer = T *;
    using table_pointer    = void **;

  private:
    union
    {
        instance_pointer instance_;
        table_pointer *vtable_;
    };

    size_t vtable_offset_;

  public:
    basic_vtable(instance_pointer instance = nullptr, size_t vtable_offset = 0)
        : instance_(instance)
        , vtable_offset_(vtable_offset)
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
        return vtable_[vtable_offset_];
    }*/

    table_pointer get() const
    {
        return vtable_[vtable_offset_];
    }

    void set(table_pointer pointer)
    {
        vtable_[vtable_offset_] = pointer;
    }

    mem_backup<table_pointer> replace(table_pointer pointer) &
    {
        return make_mem_backup(vtable_[vtable_offset_], pointer);
    }

    template <typename Q>
    mem_backup<table_pointer> replace(basic_vtable<Q> other) &
    {
        return make_mem_backup(vtable_[vtable_offset_], other.get());
    }

    unknown_vfunc_args<vtable_call<T>, T> operator[](size_t index) const
    {
        return {index, instance_, vtable_offset_};
    }
};

template <typename T>
struct vtable : basic_vtable<T>
{
    using basic_vtable<T>::basic_vtable;
    using basic_vtable<T>::operator[];

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename... Args>                                       \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, basic_vtable<T>::instance_, basic_vtable<T>::vtable_offset_}; \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <>
struct vtable<void> : basic_vtable<void>
{
    using basic_vtable::basic_vtable;
    using basic_vtable::operator[];

#define VFUNC_ACCESS(call__, __call, _call_)                                        \
    template <typename Ret, typename T, typename... Args>                           \
    vfunc<call__, Ret, T, Args...> operator[](Ret (__call T::*func)(Args...)) const \
    {                                                                               \
        return {func, instance_, vtable_offset_};                                   \
    }

    X86_CALL_MEMBER(VFUNC_ACCESS);
#undef VFUNC_ACCESS
};

template <typename T>
vtable(T *, size_t = 0) -> vtable<T>;
}