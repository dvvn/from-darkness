#pragma once
#include "internal/native_interface.h"

namespace fd
{
FD_BIND_NATIVE_INTERFACE(ISurface, vguimatsurface);

union native_gui_surface
{
    FD_NATIVE_INTERFACE(ISurface);
    function<66, void> unlock_cursor;
    function<67, void> lock_cursor;
};
}