#pragma once

namespace fd
{
struct string_view;

struct basic_variables_group
{
  protected:
    ~basic_variables_group() = default;

  public:
    virtual void on_gui()            = 0;
    virtual string_view name() const = 0;

    virtual basic_variables_group *next()
    {
        return nullptr;
    }

    virtual basic_variables_group *inner()
    {
        return nullptr;
    }
};

} // namespace fd