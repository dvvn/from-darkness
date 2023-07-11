#pragma once
#include <cstddef>

namespace fd
{
struct string_view;

template <typename T>
struct basic_variable
{
    using size_type  = size_t;
    using value_type = T;

  protected:
    ~basic_variable() = default;

  public:
    virtual string_view name() const      = 0;
    virtual char const *name_raw() const  = 0;
    virtual size_type name_length() const = 0;

    virtual value_type get() const     = 0;
    virtual void set(value_type value) = 0;
};
} // namespace fd