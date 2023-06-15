#pragma once

#include "tool/string.h"
#include "vtable.h"

namespace fd
{
struct abstract_interface : vtable<void>, noncopyable
{
    using vtable::vtable;
};

#define FD_ABSTACT_INTERFACE                                   \
    abstract_interface vtable;                                 \
    auto operator[](auto value) const->decltype(vtable[value]) \
    {                                                          \
        return vtable[value];                                  \
    }

class basic_abstract_function : public noncopyable
{
    vtable<void> table_;

  protected:
    template <typename T>
    auto func(T index) const -> decltype(table_[index])
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

template <size_t N>
struct __name
{
    [[no_unique_address]] std::false_type dummy;

    constexpr __name(const char (&str)[N])
    {
    }
};

/**
 * \brief for visualization only
 */
template <typename T, __name Name>
class named
{
    T object_;

  public:
    named(std::convertible_to<T> auto object)
        : object_((object))
    {
    }

    operator T() const
    {
        (void)Name;
        return object_;
    }
};

template <typename T, __name Name>
class named<T &, Name>
{
    T &object_;

  public:
    named(T &object)
        : object_((object))
    {
    }

    operator T &() const
    {
        (void)Name;
        return object_;
    }
};

template <typename T>
struct make_unnamed
{
    using type = T;
};

template <typename T, auto Name>
struct make_unnamed<named<T, Name>>
{
    using type = T;
};

template <size_t Index, typename Ret, typename... Args>
struct abstract_function : basic_abstract_function
{
    using basic_abstract_function::basic_abstract_function;

    Ret operator()(Args... args) const
    {
        return invoke(func(Index).get<Ret, typename make_unnamed<Args>::type...>(), args...);
    }
};

template <size_t Index, call_type_t Call, typename Ret, typename... Args>
struct abstract_function_ex : basic_abstract_function
{
    using basic_abstract_function::basic_abstract_function;

    Ret operator()(Args... args) const
    {
        return invoke(
            func<vfunc_index<Call>>(Index).template get<Ret, typename make_unnamed<Args>::type...>(), args...);
    }
};
} // namespace fd