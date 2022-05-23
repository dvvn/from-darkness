module;

#include <cheat/hooks/hook.h>

module cheat.hooks.vgui_surface.lock_cursor;
import cheat.csgo.interfaces.VguiSurface;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

CHEAT_HOOK(lock_cursor, member)
{
    lock_cursor_impl( )
    {
        this->init({&nstd::instance_of<ISurface*>, 67}, &lock_cursor_impl::callback);
    }

    void callback( ) const noexcept
    {
        /*auto thisptr = reinterpret_cast<ISurface*>(this);
        if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
            thisptr->UnlockCursor( );
        else
            CHEAT_HOOK_CALL_ORIGINAL_MEMBER( );*/

        call_original( );
    }
};

CHEAT_HOOK_IMPL(lock_cursor)