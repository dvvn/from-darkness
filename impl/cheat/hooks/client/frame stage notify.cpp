#include "frame stage notify.h"

#include "cheat/core/services loader.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/console.h"

#include "cheat/players/players list.h"

#include "cheat/netvars/config.h"

#include "cheat/sdk/IBaseClientDll.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

frame_stage_notify::frame_stage_notify()
{
	this->wait_for_service<players_list>( );
}

CHEAT_HOOK_PROXY_INIT_FN(frame_stage_notify, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(frame_stage_notify, &csgo_interfaces::client_mode, 37);

void frame_stage_notify::callback(ClientFrameStage_t stage)
{
#if !CHEAT_MODE_INGAME
	CHEAT_HOOK_PROXY_CALLBACK_BLOCKER
#else
	switch (stage)
	{
		case FRAME_UNDEFINED: break;
		case FRAME_START: break;
		case FRAME_NET_UPDATE_START: break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START: break;
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
			players_list::get_ptr( )->update( ); //todo: move to createmove
			break;
		case FRAME_NET_UPDATE_END: break;
		case FRAME_RENDER_START: break;
		case FRAME_RENDER_END: break;
		default:
			//runtime_assert("Unknown frame stage detectetd!");
			break;
	}
#endif
}

CHEAT_REGISTER_SERVICE(frame_stage_notify);
