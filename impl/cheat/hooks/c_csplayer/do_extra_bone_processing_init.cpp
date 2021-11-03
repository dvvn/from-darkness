#include "do_extra_bone_processing.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_interfaces.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"

#include <cppcoro/task.hpp>

namespace cheat::csgo
{
	class C_CSPlayer;
}

using namespace cheat;
using namespace csgo;
using namespace hooks::c_csplayer;

do_extra_bone_processing_impl::do_extra_bone_processing_impl( )
{

	this->add_dependency(csgo_interfaces::get( ));
}

nstd::address do_extra_bone_processing_impl::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_CSPlayer>( );
	const auto index           = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).add(11).deref(1).divide(4).value( );
	return (vtable.ref<nstd::address*>( )[index]);
}

auto do_extra_bone_processing_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(do_extra_bone_processing);
