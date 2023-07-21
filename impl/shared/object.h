#pragma once

namespace fd
{
class basic_stack_object
{
  protected:
    ~basic_stack_object() = default;
};

class basic_object : public basic_stack_object
{
  public:
    virtual ~basic_object() = default;
};

} // namespace fd