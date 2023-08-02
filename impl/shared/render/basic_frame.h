#pragma once

namespace fd
{
struct basic_render_frame
{
  protected:
    ~basic_render_frame() = default;

  public:
    virtual void render() const = 0;
    virtual void render_if_shown()const=0;
    
    virtual void *native_render() const = 0;
};
} // namespace fd
