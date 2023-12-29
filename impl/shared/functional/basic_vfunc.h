#pragma once

namespace fd
{
template <typename Func>
struct basic_vfunc
{
    using function_type = Func;

  protected:
    ~basic_vfunc() = default;
};

template <typename Func>
struct vfunc;
} // namespace fd