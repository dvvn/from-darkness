module;

#include "cheat/hooks/base_includes.h"
#include "cheat/players/player_includes.h"

module cheat.hooks.client:frame_stage_notify;
import cheat.players;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify( ) = default;

void frame_stage_notify::construct( ) noexcept
{
	//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).client.vfunc(32));
	const nstd::mem::basic_address vtable_holder = IBaseClientDLL::get_ptr( );
	this->set_target_method(vtable_holder.deref<1>( )[32]);
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
		players::update( ); //todo: move to createmove
		break;
	case FRAME_NET_UPDATE_END: break;
	case FRAME_RENDER_START: break;
	case FRAME_RENDER_END: break;
	default:
		//runtime_assert("Unknown frame stage detectetd!");
		break;
	}
}