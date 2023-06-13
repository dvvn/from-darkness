#pragma once

#include "vtable.h"

namespace fd
{
using abstract_interface = vtable<void>;

class basic_abstract_function
{
    vtable<void> table_;

  protected:
    template <typename T>
    auto func(T index) const
    {
        return table_[index];
    }

  public:
    /*basic_abstract_function(void *instance)
        : table_(instance)
    {
    }*/

    /*vtable<void> *operator->()
    {
        return &table_;
    }*/
};

template <size_t Index, typename Ret, typename... Args>
struct abstract_function : basic_abstract_function
{
    using basic_abstract_function::basic_abstract_function;

    Ret operator()(Args... args) const
    {
        return invoke(func(Index).get<Ret, Args...>(), args...);
    }
};

template <size_t Index, call_type_t Call, typename Ret, typename... Args>
struct abstract_function_ex : basic_abstract_function
{
    using basic_abstract_function::basic_abstract_function;

    Ret operator()(Args... args) const
    {
        return invoke(func<vfunc_index<Call>>(Index).template get<Ret, Args...>(), args...);
    }
};
}