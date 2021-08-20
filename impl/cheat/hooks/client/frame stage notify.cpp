#include "frame stage notify.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client;
using namespace utl;

utl::address frame_stage_notify::get_target_method_impl( ) const
{
	return _Pointer_to_virtual_class_table(csgo_interfaces::get_ptr( )->client.get( ))[37];
}

void frame_stage_notify::callback(ClientFrameStage_t stage)
{
	switch (stage)
	{
		case FRAME_UNDEFINED: break;
		case FRAME_START: break;
		case FRAME_NET_UPDATE_START: break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START: break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			players_list::get_ptr( )->update( );
			break;
		case FRAME_NET_UPDATE_END: break;
		case FRAME_RENDER_START: break;
		case FRAME_RENDER_END: break;
		default:
			runtime_assert("Unknown frame stage detectetd!");
			break;
	}
}
