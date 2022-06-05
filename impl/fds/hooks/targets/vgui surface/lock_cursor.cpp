module;

#include <fds/hooks/hook.h>

module fds.hooks.vgui_surface.lock_cursor;
import fds.csgo.interfaces.VguiSurface;

using namespace fds;
using namespace csgo;
using namespace hooks;
using namespace vgui_surface;

FDS_HOOK(lock_cursor, member){lock_cursor_impl(){this->init({&nstd::instance_of<ISurface*>, 67}, &lock_cursor_impl::callback);
}

void callback() const
{
    /*auto thisptr = reinterpret_cast<ISurface*>(this);
    if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
        thisptr->UnlockCursor( );
    else
        FDS_HOOK_CALL_ORIGINAL_MEMBER( );*/

    call_original();
}
}
;

FDS_HOOK_IMPL(lock_cursor)
