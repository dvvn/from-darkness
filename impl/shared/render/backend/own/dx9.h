#pragma once
#include "comptr.h"
#include "type_traits.h"
#include "render/backend/basic_dx9.h"

#include <d3d9.h>

namespace fd
{
struct own_d3d_device : noncopyable
{
    using value_type = IDirect3DDevice9;
    using pointer    = value_type *;

  private:
    comptr<IDirect3D9> d3d_;
    comptr<IDirect3DDevice9> device_;
    D3DPRESENT_PARAMETERS params_;

  public:
    own_d3d_device(HWND hwnd);

    bool resize_device(UINT w, UINT h);
    void reset_device();

    operator pointer() const;
    pointer get() const;
    pointer operator->() const;
};

class dx9_backend_own final : own_d3d_device, public basic_dx9_backend
{
    void reset() override;

  public:
    ~dx9_backend_own();
    dx9_backend_own();

    void render(ImDrawData *draw_data) override;
    void resize(UINT w, UINT h);
};
} // namespace fd