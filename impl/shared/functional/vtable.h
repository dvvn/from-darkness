#pragma once

#include "functional/vfunc.h"

namespace fd
{
template <class>
struct vtable_call_type_for : std::type_identity<default_call_type_member>
{
};

template <class Object>
using vtable_call_type = typename vtable_call_type_for<Object>::type;

template <class T>
struct vtable
{
    using instance_pointer = T*;
    using table_pointer    = void**;

  private:
    basic_vtable<T> source_;

  public:
    vtable()
        : source_(nullptr)
    {
    }

    /*explicit*/ vtable(void* instance) requires(!std::is_void_v<T>)
        : source_(static_cast<instance_pointer>(instance))
    {
    }

    vtable(instance_pointer instance)
        : source_(instance)
    {
    }

    instance_pointer operator->() const
    {
        return source_.instance();
    }

    instance_pointer instance() const
    {
        return source_.instance();
    }

    operator instance_pointer() const
    {
        return source_.instance();
    }

    /*instance_pointer operator->() const
    {
        return instance_;
    }*/

    /*operator table_pointer() const
    {
        return *vtable_;
    }*/

#if 0
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
#endif
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
} // namespace fd