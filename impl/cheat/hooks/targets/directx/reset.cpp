module;

#include <cheat/hooks/hook.h>

#include <d3d9.h>

module cheat.hooks.directx.reset;
import cheat.gui.render_interface;

using namespace cheat;
using namespace hooks;
using namespace directx;

CHEAT_HOOK(reset, member)
{
    using dummy = void;

    reset_impl()
    {
        // init({&instance_of<IDirect3DDevice9>, 16, &IDirect3DDevice9::Reset}, &reset_impl::callback);
    }

    void WINAPI callback(D3DPRESENT_PARAMETERS * params) const noexcept
    {
        gui::render_interface->ReleaseTextures();
        call_original(params);
    }
};

CHEAT_HOOK_IMPL(reset);
