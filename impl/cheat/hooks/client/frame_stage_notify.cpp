module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.client:frame_stage_notify;
import cheat.players;

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify( ) = default;

void frame_stage_notify::construct( ) noexcept
{
	this->deps( ).add<csgo_interfaces>( );
	this->deps( ).add<players_list>( );
}

bool frame_stage_notify::load( ) noexcept
{
	this->set_target_method(this->deps( ).get<csgo_interfaces>( ).client.vfunc(32));
	return hook_base::load( );
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
		this->deps( ).get<players_list>( ).update( ); //todo: move to createmove
		break;
	case FRAME_NET_UPDATE_END: break;
	case FRAME_RENDER_START: break;
	case FRAME_RENDER_END: break;
	default:
		//runtime_assert("Unknown frame stage detectetd!");
		break;
	}
}