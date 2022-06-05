#include <fds/hooks/impl.h>

#include <d3d9.h>

import fds.gui.render_interface;

static fds::function_getter _Get_present()
{
    return {&FDS_OBJECT_GET(IDirect3DDevice9), 16, &IDirect3DDevice9::Present};
}

FDS_HOOK(_Get_present(), reset, member, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    fds::gui::render_interface->ReleaseTextures();
    call_original(params);
}
