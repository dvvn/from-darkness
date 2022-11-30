#include <fd/gui/menu.h>
#include <fd/hooked/hk_vgui_surface.h>
#include <fd/valve/gui/surface.h>

#include <utility>

using namespace fd;
using namespace hooked;

lock_cursor::lock_cursor(function_getter target)
    : hook_impl("VGUI.ISurface::LockCursor")
    , hook_instance(target)
{
    // this->init({ surface, 67 }, &lock_cursor::callback);
}

void lock_cursor::callback() noexcept
{
    if (gui::Menu->visible())
    {
        const auto thisptr = reinterpret_cast<valve::gui::surface*>(this);
        if (!thisptr->IsCursorVisible())
        {
            thisptr->UnlockCursor();
            return;
        }
    }

    call_original();
}
