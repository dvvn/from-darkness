#pragma once

#include <fd/settings.h>

#include <string>

namespace fd
{
template <typename T>
class variable_impl : public basic_variable<T>
{
    T val_;

  public:
    variable_impl(T const& val)
        : val_((val))
    {
    }

    variable_impl(T&& val)
        : val_(std::move(val))
    {
    }

    T& get_ref() override
    {
        return val_;
    }

    T get() const override
    {
        return val_;
    }

    void set(T&& value) override
    {
        val_ = std::move(value);
    }

    void set(T const& value) override
    {
        val_ = value;
    }

    auto operator<=>(variable_impl const& other) const = default;
};

template <typename T>
class variable : public variable_impl<T>
{
    using variable_impl<T>::variable_impl;
};

template <>
class variable<bool> : public variable_impl<bool>
{
    using variable_impl::variable_impl;

    void toggle()
    {
        auto& val = this->get_ref();
        val       = !val;
    }
};

template <typename T>
class named_variable : public variable<T>
{
    using string_type = std::string;

    string_type name_;

  public:
    named_variable(T val, string_type name)
        : variable(std::move(val))
        , name_(std::move(name))
    {
    }

    std::string_view name() const
    {
        return { name_.begin(), name_.end() };
    }
};

}