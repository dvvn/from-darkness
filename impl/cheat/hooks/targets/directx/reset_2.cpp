
#include <cheat/hooks/impl.h>

#include <d3d9.h>

import cheat.gui.context;
import cheat.gui.render_interface;

static cheat::function_getter _Get_reset()
{
    return {&CHEAT_OBJECT_GET(IDirect3DDevice9), 17, &IDirect3DDevice9::Reset};
}

CHEAT_HOOK(_Get_reset(), present, member, HRESULT WINAPI, THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    using namespace cheat::gui;
    render_interface->RenderContext(&context);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
