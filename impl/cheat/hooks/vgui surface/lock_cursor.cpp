module;


module cheat.hooks.vgui_surface:lock_cursor;
import cheat.gui;
import nstd.mem.address;

using namespace cheat;
using namespace hooks::vgui_surface;

lock_cursor::lock_cursor( )
{
	//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).vgui_surface.vfunc(67));
	this->set_target_method(nstd::mem::basic_address(csgo::ISurface::get_ptr( )).deref<1>( )[67]);
}

void lock_cursor::callback( )
{
	auto inst = this->get_object_instance( );
	if (!inst->IsCursorVisible( ) && gui::menu::visible( ))
	{
		this->store_return_value( );
		inst->UnlockCursor( );
	}
}