#pragma once

namespace fd
{
struct renderable_inner
{
    virtual ~renderable_inner() = default;

  protected:
    virtual bool begin_frame() = 0;
    virtual void end_frame()   = 0;
};

struct renderable
{
    virtual ~renderable() = default;
    virtual bool render() = 0;
};

} // namespace fd