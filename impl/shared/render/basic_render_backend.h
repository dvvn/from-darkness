#pragma once

#include "basic_backend.h"

// ReSharper disable once CppInconsistentNaming
struct ImDrawData;

namespace fd
{
struct basic_render_backend : basic_backend
{
  protected:
    ~basic_render_backend() = default;

  public:
    virtual void render(ImDrawData *draw_data) = 0;
    virtual void reset()                       = 0;
};
}