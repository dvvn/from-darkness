module;

#include <fd/object.h>

#include <d3d9.h>

export module fd.valve.d3d9;

FD_OBJECT(d3d_device9, IDirect3DDevice9);

export namespace fd::valve
{
    using ::d3d_device9;
}
