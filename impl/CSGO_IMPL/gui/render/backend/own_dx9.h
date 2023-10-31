#pragma once

#include "comptr.h"
#include "noncopyable.h"
#include "gui/render/backend/basic_dx9.h"
#include "gui/render/backend/basic_win32.h"

#include <d3d9.h>

namespace fd::gui
{
class own_dx9_backend_data : public noncopyable
{
    friend class own_dx9_backend;

    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

    own_dx9_backend_data(HWND hwnd);
};

class own_dx9_backend final : own_dx9_backend_data, public basic_dx9_backend
{
    void reset();

  public:
    own_dx9_backend(HWND hwnd);

    using basic_dx9_backend::new_frame;
    void render(ImDrawData* draw_data);
    void resize(simple_win32_window_size const& size);
};
} // namespace fd::gui