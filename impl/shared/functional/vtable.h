#pragma once

#include "mem_backup.h"
#include "functional/vfunc.h"

namespace fd
{
template <class>
struct vtable_call_type_for : std::type_identity<default_call_type_member>
{
};

template <class Object>
using vtable_call_type = typename vtable_call_type_for<Object>::type;

template <typename Fn, size_t Index>
struct vfunc_tag
{
    Fn fn;
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
};

template <typename T>
vtable(T*, size_t = 0) -> vtable<T>;

template <typename Func, typename T>
auto get(Func fn, vtable<T> table) -> vfunc<Func>
{
    return {fn, table.instance()};
}

template <typename Func, typename T>
auto get(Func fn, vtable<T> table, std::in_place_t hint) -> vfunc<Func>
{
    return {fn, table.instance(), hint};
}

template <typename Func, typename T>
auto get(size_t index, vtable<T> table) -> vfunc<Func>
{
    return {index, table.instance()};
}
}