
#include <fd/hooks/impl.h>

import fd.valve.vgui_surface;
import fd.gui.menu;

using namespace fd;
using namespace fd::valve;

FD_HOOK_VTABLE(vgui_surface, lock_cursor, 67, void)
{
    auto thisptr = reinterpret_cast<vgui_surface*>(this);
    if (!thisptr->IsCursorVisible() && gui::menu->shown())
        thisptr->UnlockCursor( );
    else
        call_original();
}
