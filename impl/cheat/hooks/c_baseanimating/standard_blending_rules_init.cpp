#include "standard_blending_rules.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/netvars.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

standard_blending_rules_impl::standard_blending_rules_impl( )
{
	CHEAT_SERVICE_ADD_SHARED_DEPENDENCY(netvars);
}

nstd::address standard_blending_rules_impl::get_target_method_impl( ) const
{
	const nstd::address vtable = csgo_modules::client.find_vtable<C_BaseAnimating>( );
	const auto index           = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC">( ).add(11).deref(1).divide(4).value( );
	return (vtable.ref<nstd::address*>( )[index]);
}

auto standard_blending_rules_impl::load_impl( ) noexcept -> load_result
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(standard_blending_rules);
