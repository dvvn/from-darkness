module;

#include <cheat/hooks/instance.h>
#include "cheat/players/player_includes.h"

module cheat.hooks.client.frame_stage_notify;
import cheat.players;
import cheat.csgo.interfaces.BaseClient;
import cheat.hooks.base;
import nstd.mem.address;

using namespace cheat;
using namespace csgo;
using namespace hooks;

CHEAT_HOOK_INSTANCE(client, frame_stage_notify);

static void* target( ) noexcept
{
	const nstd::mem::basic_address<void> vtable_holder = &nstd::instance_of<IBaseClientDLL*>;
	return vtable_holder.deref<1>( )[32];
}

struct replace
{
	void fn(ClientFrameStage_t stage) noexcept
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

		CHEAT_HOOK_CALL_ORIGINAL_MEMBER(stage);
	}
};

CHEAT_HOOK_INIT(client, frame_stage_notify);