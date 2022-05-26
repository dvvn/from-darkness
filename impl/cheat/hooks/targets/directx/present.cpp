module;

#include <cheat/hooks/impl.h>

#include <d3d9.h>

module cheat.hooks.directx.present;
import cheat.gui.context;
import cheat.gui.render_interface;

using namespace cheat;
using namespace hooks;
using namespace directx;

#define ARGS_T THIS_ CONST RECT *source_rect, CONST RECT *desc_rect, HWND dest_window_override, CONST RGNDATA *dirty_region

CHEAT_HOOK_BODY(present, member, HRESULT WINAPI, ARGS_T);

CHEAT_HOOK_INIT(present)
{
    hook::init({&CHEAT_OBJECT_GET(IDirect3DDevice9), 17, &IDirect3DDevice9::Present}, &present_impl::callback);
}

CHEAT_HOOK_CALLBACK(present, HRESULT WINAPI, ARGS_T)
{
    using namespace gui;
    render_interface->RenderContext(&context);
    return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
}
