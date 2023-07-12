#pragma once

namespace fd
{
struct basic_render_frame
{
  protected:
    ~basic_render_frame() = default;

  public:
    virtual void render() = 0;
};
} // namespace fd