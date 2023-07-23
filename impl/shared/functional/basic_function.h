#pragma once

namespace fd
{
template <typename Ret, typename... Args>
class basic_function
{
  protected:
    ~basic_function() = default;

  public:
    virtual Ret operator()(Args... args) const = 0;
};
} // namespace fd
