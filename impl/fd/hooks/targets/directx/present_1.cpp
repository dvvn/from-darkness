#include <fd/hooks/impl.h>

#include <d3d9.h>

import fd.gui.basic_render_interface;

using namespace fd;

static function_getter _Get_target()
{
    return { &FD_OBJECT_GET(IDirect3DDevice9), 16 };
}

FD_HOOK(_Get_target(), member, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    gui::render_interface->release_textures();
    call_original(params);
}
