#include "should_skip_animation_frame.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/netvars.h"

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

should_skip_animation_frame::should_skip_animation_frame( )
{
	this->wait_for_service<netvars>( );
}

nstd::address should_skip_animation_frame::get_target_method_impl( ) const
{
	return csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">( );
}

service_impl::load_result should_skip_animation_frame::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_REGISTER_SERVICE(should_skip_animation_frame);
