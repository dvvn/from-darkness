module;

#include <utility>

module fd.hooks.vgui_surface;
import fd.gui.menu;
import fd.valve.vgui_surface;

using namespace fd;
using namespace hooks;

lock_cursor::lock_cursor(function_getter target)
    : impl("VGUI.ISurface::LockCursor")
    , instance(target)
{
    // this->init({ surface, 67 }, &lock_cursor::callback);
}

void lock_cursor::callback()
{
    auto thisptr = reinterpret_cast<valve::vgui_surface*>(this);
    if (!thisptr->IsCursorVisible() && gui::menu->visible())
        thisptr->UnlockCursor();
    else
        call_original();
}
