#pragma once

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_context
{
  protected:
    ~basic_render_context() = default;

  public:
    virtual void begin_scene()          = 0;
    virtual void end_scene()            = 0;
    virtual ImDrawData *data() = 0;
};
} // namespace fd