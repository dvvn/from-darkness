#include "should_interpolate.h"

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

should_interpolate::should_interpolate( )
{
	this->wait_for_service<netvars>( );
}

nstd::address should_interpolate::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_BaseEntity>( );
	const auto index           = csgo_modules::client.find_signature<"8B 06 8B CE 8B 80 ? ? 00 00 FF D0 84 C0 74 5C">( ).add(6).deref(1).divide(4).value( );
	return vtable.ref<ptrdiff_t*>( )[index];
}

service_impl::load_result should_interpolate::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(should_interpolate);
