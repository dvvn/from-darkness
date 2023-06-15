#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
union vgui_surface
{
    FD_ABSTRACT_INTERFACE(vgui_surface);
    abstract_function<66, void> unlock_cursor;
    abstract_function<67, void> lock_cursor;
};
}