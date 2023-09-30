#pragma once
#include "render/basic_render_backend.h"

// ReSharper disable once CppInconsistentNaming
struct IDirect3DDevice9;

namespace fd
{
struct basic_dx9_backend : basic_render_backend
{
    virtual void setup(IDirect3DDevice9* device);
    void destroy() override;
    void new_frame() override;
    void render(ImDrawData* draw_data) override;
    void reset() override;
};
}