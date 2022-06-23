
#include <fd/hooks/impl.h>

import fd.csgo.interfaces.VguiSurface;

using namespace fd;
using namespace csgo;

static function_getter _Get_target()
{
    return { &FD_OBJECT_GET(ISurface), 67 };
}

FD_HOOK(_Get_target(), member, void)
{
    /*auto thisptr = reinterpret_cast<ISurface*>(this);
    if(!thisptr->IsCursorVisible( ) && gui::menu::visible( ))
        thisptr->UnlockCursor( );
    else
        call_original( );*/

    call_original();
}
