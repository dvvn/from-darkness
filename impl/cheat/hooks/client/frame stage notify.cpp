#include "frame stage notify.h"

#include "cheat/core/csgo interfaces.h"
#include "cheat/players/players list.h"

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace client;
using namespace utl;

frame_stage_notify::frame_stage_notify( )
{
}

bool frame_stage_notify::Do_load( )
{
	this->target_func_ = method_info::make_member_virtual(csgo_interfaces::get_ptr( )->client.get( ), 37);

	this->hook( );
	this->enable( );

	return true;
}

void frame_stage_notify::Callback(ClientFrameStage_t stage)
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
