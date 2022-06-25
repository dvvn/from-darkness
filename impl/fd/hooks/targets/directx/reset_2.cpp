
#include <fd/hooks/impl.h>

#include <d3d9.h>

import fd.gui.context;
import fd.gui.render_interface;

using namespace fd;

static function_getter _Get_target()
{
    return { &FD_OBJECT_GET(IDirect3DDevice9), 17 };
}

FD_HOOK(_Get_target(), member, HRESULT WINAPI, THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    gui::render_interface->RenderContext(&gui::context);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}