#pragma once

#include "basic_variable.h"
#include "string/view.h"

namespace fd
{
template <typename T>
struct variable_range
{
    T min;
    T max;
};

template <>
struct variable_range<bool>
{
};

template <typename T>
class variable : public basic_variable<T>
{
    using value_type  = T;
    using string_type = string_view;
    using range_type  = variable_range<T>;
    using size_type   = typename basic_variable<T>::size_type;

    string_type name_;
    value_type value_;
    range_type range_;

  public:
    variable(string_view name, value_type value, range_type range = {})
        : name_(std::move(name))
        , value_(std::move(value))
        , range_(std::move(range))
    {
    }

    string_view name() const override
    {
        return name_;
    }

    char const *name_raw() const override
    {
        return name_.data();
    }

    size_type name_length() const override
    {
        return name_.length();
    }

    value_type get() const override
    {
        return value_;
    }

    void set(value_type value) override
    {
        value_ = std::move(value);
    }

    //----

    operator value_type &()
    {
        return value_;
    }

    operator value_type const &() const
    {
        return value_;
    }

    value_type *operator&()
    {
        return &value_;
    }

    range_type const &range() const
    {
        return range_;
    }
};
} // namespace fd