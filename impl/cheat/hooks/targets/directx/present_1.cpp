#include <cheat/hooks/impl.h>

#include <d3d9.h>

import cheat.gui.render_interface;

static cheat::function_getter _Get_present()
{
    return {&CHEAT_OBJECT_GET(IDirect3DDevice9), 16, &IDirect3DDevice9::Present};
}

CHEAT_HOOK(_Get_present(), reset, member, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    cheat::gui::render_interface->ReleaseTextures();
    call_original(params);
}
