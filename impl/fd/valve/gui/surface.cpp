#include <fd/utils/functional.h>
#include <fd/valve/gui/surface.h>

namespace fd::valve::gui
{
bool surface::IsCursorVisible() const
{
    return vfunc<bool>(this, 58);
}

void surface::UnlockCursor()
{
    vfunc<void>(this, 66);
}

void surface::LockCursor()
{
    vfunc<void>(this, 67);
}
}