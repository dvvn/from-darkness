module;

#include <utility>

module fd.hooks.vgui_surface;
import fd.gui.menu;
import fd.functional.invoke;
import fd.valve.vgui_surface;

using namespace fd;
using namespace hooks;

static lock_cursor* _Lock_cursor;

lock_cursor::lock_cursor(function_getter target)
    : impl("VGUI.ISurface::LockCursor")
{
    // this->init({ surface, 67 }, &lock_cursor::callback);
    this->init(target, &lock_cursor::callback);
    _Lock_cursor = this;
}

lock_cursor::lock_cursor(lock_cursor&& other)
    : impl(std::move(other))
{
    _Lock_cursor = this;
}

void lock_cursor::callback()
{
    auto thisptr = reinterpret_cast<valve::vgui_surface*>(this);
    if (!thisptr->IsCursorVisible() && gui::menu->visible())
        thisptr->UnlockCursor();
    else
        invoke(&lock_cursor::callback, _Lock_cursor->get_original_method(), this);
}
