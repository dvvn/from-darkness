

#include <fd/hooks/impl.h>

#include <d3d9.h>

import fd.gui.basic_render_interface;
import fd.rt_modules;

using namespace fd;

FD_HOOK_VTABLE(IDirect3DDevice9, present, 17, HRESULT WINAPI, THIS_ CONST RECT* source_rect, CONST RECT* desc_rect, HWND dest_window_override, CONST RGNDATA* dirty_region)
{
    fd::invoke(gui::render_interface);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
