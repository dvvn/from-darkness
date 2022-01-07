module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.client:frame_stage_notify;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify( )
{
	this->add_dependency(players_list::get( ));
}

void* frame_stage_notify::get_target_method( ) const
{
	return csgo_interfaces::get( )->client.vfunc(37).ptr( );
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
		players_list::get( )->update( ); //todo: move to createmove
		break;
	case FRAME_NET_UPDATE_END: break;
	case FRAME_RENDER_START: break;
	case FRAME_RENDER_END: break;
	default:
		//runtime_assert("Unknown frame stage detectetd!");
		break;
	}
}

CHEAT_SERVICE_REGISTER_GAME(frame_stage_notify);
