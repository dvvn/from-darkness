module;

#include <utility>

module fd.hooks.vgui_surface;
// import fd.gui.menu;
import fd.functional.invoke;

using namespace fd;
using namespace hooks;

/* FD_HOOK_VTABLE(vgui_surface, lock_cursor, 67, void)
{
    auto thisptr = reinterpret_cast<vgui_surface*>(this);
    if (!thisptr->IsCursorVisible() && gui::menu->shown())
        thisptr->UnlockCursor();
    else
        call_original();
} */

static lock_cursor* _Lock_cursor;

lock_cursor::~lock_cursor()
{
    // added for logging only
    if (*this)
        impl::disable();
}

lock_cursor::lock_cursor(valve::vgui_surface* surface)
{
    this->init({ surface, 67 }, &lock_cursor::callback);
    _Lock_cursor = this;
}

lock_cursor::lock_cursor(lock_cursor&& other)
    : impl(std::move(other))
{
    _Lock_cursor = this;
}

string_view lock_cursor::name() const
{
    return "VGUI.ISurface::LockCursor";
}

void lock_cursor::callback()
{
    /* auto thisptr = reinterpret_cast<vgui_surface*>(this);
    if (!thisptr->IsCursorVisible() && gui::menu->shown())
        thisptr->UnlockCursor();
    else */
    invoke(&lock_cursor::callback, _Lock_cursor->get_original_method(), this);
}
