#pragma once

namespace fd
{
class basic_backend
{
  protected:
    ~basic_backend() = default;

  public:
    virtual void destroy()   = 0;
    virtual void new_frame() = 0;
};
} // namespace fd