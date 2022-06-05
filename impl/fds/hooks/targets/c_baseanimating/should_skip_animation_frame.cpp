module;

#include <fds/hooks/hook.h>

module fds.hooks.c_base_animating.should_skip_animation_frame;
import fds.csgo.interfaces.C_BaseAnimating;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;
using namespace hooks;
using namespace c_base_animating;

FDS_HOOK(should_skip_animation_frame, member){should_skip_animation_frame_impl(){init(csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">(), &should_skip_animation_frame_impl::callback);
}

void callback() const
{
    call_original();
}
}
;

FDS_HOOK_IMPL(should_skip_animation_frame);
