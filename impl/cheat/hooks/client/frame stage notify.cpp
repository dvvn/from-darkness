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
	this->Wait_for<players_list>( );
}

void frame_stage_notify::Load( )
{
#ifndef CHEAT_GUI_TEST
	this->target_func_ = method_info::make_member_virtual<IBaseClientDLL*>(csgo_interfaces::get( ).client, 37);

	this->hook( );
	this->enable( );
#endif
}

utl::string frame_stage_notify::Get_loaded_message( ) const
{
#ifndef CHEAT_GUI_TEST
		return service_base::Get_loaded_message( );
#else
	return Get_loaded_message_disabled( );
#endif
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
			players_list::get( ).update( );
			break;
		case FRAME_NET_UPDATE_END: break;
		case FRAME_RENDER_START: break;
		case FRAME_RENDER_END: break;
		default:
			BOOST_ASSERT("Unknown frame stage detectetd!");
			break;
	}
}
