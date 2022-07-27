
#include <fd/hooks/impl.h>

#include <d3d9.h>

import fd.gui.basic_render_interface;

using namespace fd;

FD_HOOK_VTABLE(IDirect3DDevice9, reset, 16, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    gui::render_interface->release_textures();
    call_original(params);
}
