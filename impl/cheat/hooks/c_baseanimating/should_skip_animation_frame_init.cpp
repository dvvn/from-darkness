#include "should_skip_animation_frame.h"

#include "cheat/core/console.h"
#include "cheat/core/csgo_modules.h"
#include "cheat/core/services_loader.h"
#include "cheat/netvars/config.h"
#include "cheat/netvars/netvars.h"

using namespace cheat;
using namespace hooks::c_base_animating;

should_skip_animation_frame::should_skip_animation_frame( )
{
	this->wait_for_service<netvars>( );
}

CHEAT_HOOK_PROXY_INIT_FN(should_skip_animation_frame, CHEAT_MODE_INGAME)
CHEAT_HOOK_PROXY_TARGET_FN(should_skip_animation_frame, CHEAT_MODE_INGAME, csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">())

CHEAT_REGISTER_SERVICE(should_skip_animation_frame);
