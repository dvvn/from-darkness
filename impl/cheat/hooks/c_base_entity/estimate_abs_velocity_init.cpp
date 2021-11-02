#include "estimate_abs_velocity.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/netvars.h"

namespace cheat::csgo
{
	class C_BaseEntity;
}

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_entity;

estimate_abs_velocity_impl::estimate_abs_velocity_impl( )
{
	this->add_dependency(netvars::get( ));
}

nstd::address estimate_abs_velocity_impl::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index           = csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">( ).add(2).deref(1).divide(4).value( );
	return (vtable.ref<nstd::address*>( )[index]);
}

basic_service::load_result estimate_abs_velocity_impl::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(estimate_abs_velocity);
