#pragma once

namespace fd
{
class basic_stack_object
{
  protected:
    ~basic_stack_object() = default;
};

struct basic_object : basic_stack_object
{
    virtual ~basic_object() = default;
};

} // namespace fd