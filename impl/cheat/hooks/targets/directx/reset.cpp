module;

#include <cheat/hooks/impl.h>

#include <d3d9.h>

module cheat.hooks.directx.reset;
import cheat.gui.render_interface;

using namespace cheat;
using namespace hooks;
using namespace directx;

CHEAT_HOOK_BODY(reset, member, void WINAPI, D3DPRESENT_PARAMETERS* params);

CHEAT_HOOK_INIT(reset)
{
    hook::init({&CHEAT_OBJECT_GET(IDirect3DDevice9), 16, &IDirect3DDevice9::Reset}, &reset_impl::callback);
}

CHEAT_HOOK_CALLBACK(reset, void WINAPI, D3DPRESENT_PARAMETERS* params)
{
    gui::render_interface->ReleaseTextures();
    call_original(params);
}
