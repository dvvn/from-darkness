#include <fd/hooks/impl.h>

#include <d3d9.h>

import fd.gui.render_interface;

static fd::function_getter _Get_present()
{
    return {&FD_OBJECT_GET(IDirect3DDevice9), 16, &IDirect3DDevice9::Present};
}

FD_HOOK(_Get_present(), reset, member, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    fd::gui::render_interface->ReleaseTextures();
    call_original(params);
}
