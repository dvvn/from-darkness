#include <fd/valve/gui/surface.h>
#include <fd/vfunc.h>

namespace fd::valve::gui
{
bool surface::IsCursorVisible() const
{
    return vtable(this).call<bool>(58);
}

void surface::UnlockCursor()
{
    return vtable(this).call(66);
}

void surface::LockCursor()
{
    return vtable(this).call(67);
}
}