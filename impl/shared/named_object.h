#pragma once

namespace fd
{
struct string_view;

class basic_named_object
{
  protected:
    ~basic_named_object() = default;

  public:
    virtual string_view name() const = 0;
};
} // namespace fd
