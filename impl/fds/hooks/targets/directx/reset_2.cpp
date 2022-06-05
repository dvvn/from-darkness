
#include <fds/hooks/impl.h>

#include <d3d9.h>

import fds.gui.context;
import fds.gui.render_interface;

static fds::function_getter _Get_reset()
{
    return {&FDS_OBJECT_GET(IDirect3DDevice9), 17, &IDirect3DDevice9::Reset};
}

FDS_HOOK(_Get_reset(), present, member, HRESULT WINAPI, THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    using namespace fds::gui;
    render_interface->RenderContext(&context);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
