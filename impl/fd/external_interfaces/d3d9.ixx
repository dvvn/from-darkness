module;

#include <fd/core/object.h>

#include <d3d9.h>

export module fd.d3d9;

FD_OBJECT(d3d_device9, IDirect3DDevice9);

export namespace fd
{
    using ::d3d_device9;
}
