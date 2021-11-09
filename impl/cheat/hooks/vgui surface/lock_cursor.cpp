#include "lock_cursor.h"

#include "cheat/gui/menu.h"

#include "cheat/csgo/ISurface.hpp"

using namespace cheat;
using namespace hooks::vgui_surface;

void lock_cursor_impl::callback( )
{
    auto inst = this->get_object_instance( );
    if (!inst->IsCursorVisible( ) && gui::menu::get( )->visible( ))
    {
        this->store_return_value( );
        inst->UnlockCursor( );
    }
}
