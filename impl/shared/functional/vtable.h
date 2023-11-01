#pragma once

#include "mem_backup.h"
#include "functional/vfunc.h"

namespace fd
{
template <class T>
inline constexpr auto vtable_call_type = default_call_type_member;

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
struct vtable
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

    template <typename Func>
    auto operator[](Func fn) const -> typename function_info<Func>::template rebind<vfunc>
    {
        return {fn, instance_};
    }

    template <call_type Call_T>
    auto operator[](vfunc_index<Call_T> index) const -> unknown_vfunc_args<Call_T, T>
    {
        return {index, instance_};
    }

    auto operator[](ptrdiff_t index) const -> unknown_vfunc_args<vtable_call_type<T>, T>
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

// template<call_type_t Call_T,typename member_function,class T,typename ...Args>
// class vfunc_view
//{
//     vtable<T>
// };
}