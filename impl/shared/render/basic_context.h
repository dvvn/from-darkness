#pragma once

namespace fd
{
struct basic_render_context
{
  protected:
    ~basic_render_context() = default;

  public:
    virtual void begin_scene() = 0;
    virtual void end_scene()   = 0;
};
} // namespace fd