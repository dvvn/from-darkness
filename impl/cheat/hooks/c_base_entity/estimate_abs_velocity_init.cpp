#include "estimate_abs_velocity.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

namespace cheat::csgo
{
	class C_BaseEntity;
}

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_entity;

estimate_abs_velocity::estimate_abs_velocity( )
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(estimate_abs_velocity, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(estimate_abs_velocity, CHEAT_MODE_INGAME,
						   csgo_modules::client.find_vtable<C_BaseEntity>(),
						   csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">().add(2).deref(1).divide(4));

CHEAT_REGISTER_SERVICE(estimate_abs_velocity);
