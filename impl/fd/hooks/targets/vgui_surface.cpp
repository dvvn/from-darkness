
#include <fd/hooks/impl.h>

import fd.valve.vgui_surface;

using namespace fd;
using namespace fd::valve;

FD_HOOK_VTABLE(vgui_surface, lock_cursor, 67, void)
{
    /*auto thisptr = reinterpret_cast<surface*>(this);
    if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
        thisptr->UnlockCursor( );
    else
        call_original( );*/

    call_original();
}
