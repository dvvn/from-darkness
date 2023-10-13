#pragma once

#include "basic_dx9.h"
//
#include "comptr.h"
#include "noncopyable.h"

#include <d3d9.h>

namespace fd
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
    void resize(UINT w, UINT h);
    void* native() const;
};
} // namespace fd