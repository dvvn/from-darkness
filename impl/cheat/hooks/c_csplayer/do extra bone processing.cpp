#include "do extra bone processing.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo modules.h"
#include "cheat/core/csgo interfaces.h"
#include "cheat/core/services loader.h"

#include "cheat/netvars/config.h"

namespace cheat::csgo
{
	class C_CSPlayer;
}

using namespace cheat;
using namespace hooks;
using namespace c_csplayer;
using namespace csgo;

using namespace nstd::address_pipe;

do_extra_bone_processing::do_extra_bone_processing()
{
	this->wait_for_service<csgo_interfaces>( );
}

CHEAT_HOOK_PROXY_INIT_FN(do_extra_bone_processing, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(do_extra_bone_processing, CHEAT_MODE_INGAME,
						   CHEAT_FIND_VTABLE(client, C_CSPlayer),
						   CHEAT_FIND_SIG(client, "8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC", add(11), deref(1), divide(4), value));

void do_extra_bone_processing::callback([[maybe_unused]] CStudioHdr* studio_hdr, [[maybe_unused]] Vector pos[],
										[[maybe_unused]] Quaternion q[], [[maybe_unused]] matrix3x4a_t bone_to_world[],
										[[maybe_unused]] CBoneBitList& bone_computed, [[maybe_unused]] CIKContext* ik_context)
{
#if !CHEAT_MODE_INGAME
	CHEAT_CALL_BLOCKER
#else
	this->return_value_.set_original_called( );

	//DoProceduralFootPlant also skipped here
#endif
}

CHEAT_REGISTER_SERVICE(do_extra_bone_processing);
