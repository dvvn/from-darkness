module;

#include <fds/core/object.h>

#include <d3d9.h>

export module fds.d3d_device9;

class d3d_device9_setter
{
    IDirect3DDevice9* ptr_;

  public:
    d3d_device9_setter(IDirect3DDevice9* const ptr = nullptr);
    IDirect3DDevice9* operator&() const;
    IDirect3DDevice9* operator->() const;
};

FDS_OBJECT(d3d_device9, d3d_device9_setter);
FDS_OBJECT(d3d_device9_raw, IDirect3DDevice9);

export namespace fds
{
    using ::d3d_device9;
    using ::d3d_device9_raw;
} // namespace fds
