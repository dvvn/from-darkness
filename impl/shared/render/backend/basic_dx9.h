﻿#pragma once
#include "render/basic_render_backend.h"

// ReSharper disable once CppInconsistentNaming
struct IDirect3DDevice9;

namespace fd
{
class basic_dx9_backend : public basic_render_backend
{
  protected:
    ~basic_dx9_backend() = default;
    basic_dx9_backend(IDirect3DDevice9 *device);

  public:
    void destroy() override;
    void new_frame() override;
    void render(ImDrawData *draw_data) override;
    void reset() override;
};
}