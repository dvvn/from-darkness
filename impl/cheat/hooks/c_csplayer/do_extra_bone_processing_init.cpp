#include "do_extra_bone_processing.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"

namespace cheat::csgo
{
	class C_CSPlayer;
}

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

do_extra_bone_processing::do_extra_bone_processing( )
{
	this->wait_for_service<csgo_interfaces>( );
}

CHEAT_HOOK_PROXY_INIT_FN(do_extra_bone_processing, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(do_extra_bone_processing, CHEAT_MODE_INGAME,
						   csgo_modules::client.find_vtable<C_CSPlayer>(),
						   csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">().add(11).deref(1).divide(4).value());

CHEAT_REGISTER_SERVICE(do_extra_bone_processing);
