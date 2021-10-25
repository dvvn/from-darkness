#include "standard_blending_rules.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

standard_blending_rules::standard_blending_rules( )
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(standard_blending_rules, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(standard_blending_rules, CHEAT_MODE_INGAME,
						   csgo_modules::client.find_vtable<C_BaseAnimating>(),
						   csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC">().add(11).deref(1).divide(4).value());

CHEAT_REGISTER_SERVICE(standard_blending_rules);
