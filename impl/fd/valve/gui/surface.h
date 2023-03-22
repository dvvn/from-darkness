#pragma once

namespace fd::valve::gui
{
class surface
{
    void *vtable_;

  public:
    bool IsCursorVisible() const;
    void UnlockCursor();
    void LockCursor();
};
}