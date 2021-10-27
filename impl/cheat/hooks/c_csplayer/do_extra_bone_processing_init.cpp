#include "do_extra_bone_processing.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"

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

nstd::address do_extra_bone_processing::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_CSPlayer>( );
	const auto index           = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).add(11).deref(1).divide(4).value( );
	return vtable.ref<ptrdiff_t*>( )[index];
}

service_impl::load_result do_extra_bone_processing::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(do_extra_bone_processing);
