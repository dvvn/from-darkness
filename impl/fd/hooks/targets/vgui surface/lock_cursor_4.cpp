
#include <fd/hooks/impl.h>

import fd.valve.vgui_surface;

using namespace fd;
using namespace fd::valve;

static function_getter _Get_target()
{
    return { &FD_OBJECT_GET(vgui_surface), 67 };
}

FD_HOOK(_Get_target(), member, void)
{
    /*auto thisptr = reinterpret_cast<surface*>(this);
    if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
        thisptr->UnlockCursor( );
    else
        call_original( );*/

    call_original();
}
