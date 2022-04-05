module;

#include <cheat/hooks/instance.h>
#include "cheat/players/player_includes.h"

module cheat.hooks.client.frame_stage_notify;
import cheat.players;
import cheat.csgo.interfaces.BaseClient;
import cheat.hooks.base;
import nstd.one_instance;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

using frame_stage_notify_base = hooks::base<void(IBaseClientDLL::*)(ClientFrameStage_t)>;
struct frame_stage_notify_impl :frame_stage_notify_base
{
	frame_stage_notify_impl( )
	{
		//this->set_target_method(this->deps( ).get<csgo_interfaces>( ).client.vfunc(32));
		const nstd::mem::basic_address vtable_holder = IBaseClientDLL::get_ptr( );
		this->set_target_method(vtable_holder.deref<1>( )[32]);
	}

	void callback(ClientFrameStage_t stage)
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
};

CHEAT_HOOK_INSTANCE(client, frame_stage_notify);