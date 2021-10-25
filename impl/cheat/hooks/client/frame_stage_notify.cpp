#include "frame_stage_notify.h"

#include "cheat/players/players_list.h"

#include "cheat/csgo/IBaseClientDll.hpp"

using namespace cheat;
using namespace csgo;
using namespace hooks::client;

void frame_stage_notify::callback(ClientFrameStage_t stage)
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
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
