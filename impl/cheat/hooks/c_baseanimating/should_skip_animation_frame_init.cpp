#include "should_skip_animation_frame.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/netvars.h"

#include <cppcoro/task.hpp>

using namespace cheat;
using namespace csgo;
using namespace hooks::c_base_animating;

should_skip_animation_frame_impl::should_skip_animation_frame_impl( )
{
	this->add_dependency(netvars::get());
}

void* should_skip_animation_frame_impl::get_target_method( ) const
{
	const auto addr = csgo_modules::client->find_signature("57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02");
	return addr.ptr( );
}

basic_service::load_result should_skip_animation_frame_impl::load_impl( ) noexcept
{
	CHEAT_LOAD_HOOK_PROXY;
}

CHEAT_SERVICE_REGISTER(should_skip_animation_frame);
