#pragma once

namespace fd
{
template <typename Ret, typename... Args>
struct basic_function
{
    using return_type = Ret;

  protected:
    ~basic_function() = default;

  public:
    virtual Ret operator()(Args... args) const = 0;
};
} // namespace fd
