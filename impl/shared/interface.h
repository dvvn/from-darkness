#pragma once

namespace fd
{
class basic_stack_interface
{
  protected:
    ~basic_stack_interface() = default;
};

class basic_interface //: public basic_stack_interface
{
  public:
    virtual ~basic_interface() = default;
};

} // namespace fd