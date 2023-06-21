#pragma once
#include <fd/abstract_interface.h>

namespace fd::valve
{
// ReSharper disable once CppInconsistentNaming
class ISurface;

union vgui_surface
{
    FD_ABSTRACT_INTERFACE(ISurface);
    abstract_function<66, void> unlock_cursor;
    abstract_function<67, void> lock_cursor;
};
}