module;

#include <cheat/hooks/hook.h>

#include <d3d9.h>

module cheat.hooks.directx.present;
import cheat.gui.context;
import cheat.gui.render_interface;

using namespace cheat;
using namespace hooks;
using namespace directx;

CHEAT_HOOK(present, member)
{
    present_impl( )
    {
        init({&instance_of<IDirect3DDevice9>, 17, &IDirect3DDevice9::Present}, &present_impl::callback);
    }

    HRESULT WINAPI callback(THIS_ CONST RECT * source_rect, CONST RECT * desc_rect, HWND dest_window_override, CONST RGNDATA * dirty_region) const noexcept
    {
        using namespace gui;
        instance_of<render_interface>->RenderContext(*instance_of<context>);
        return call_original(source_rect, desc_rect, dest_window_override, dirty_region);
    }
};

CHEAT_HOOK_IMPL(present);
