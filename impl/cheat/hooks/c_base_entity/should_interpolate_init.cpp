#include "should_interpolate.h"

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

//utils::find_signature\("([a-z]+).*,.("[A-F0-9 ]+")
//find_signature\("([a-z0-9]+).*,.(".*")
//csgo_modules::$1.find_signature<$2>(
//
//utils::vtable_pointer<(.*)>\("([a-z0-9]+).*\)
//csgo_modules::$2.find_vtable<$1>()

should_interpolate::should_interpolate( )
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(should_interpolate, FALSE)
CHEAT_HOOK_PROXY_TARGET_FN(should_interpolate, FALSE,
						   csgo_modules::client.find_vtable<C_BaseEntity>(),
						   csgo_modules::client.find_signature<"8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C">( ).add(6).deref(1).divide(4).value());

CHEAT_REGISTER_SERVICE(should_interpolate);
