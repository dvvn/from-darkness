module;

#include <cheat/hooks/hook.h>

module cheat.hooks.c_base_animating.should_skip_animation_frame;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_base_animating;

CHEAT_HOOK(should_skip_animation_frame, member)
{
    should_skip_animation_frame_impl( )
    {
        init(csgo_modules::client.find_signature<"57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02">( ), &should_skip_animation_frame_impl::callback);
    }

    void callback() const
    {
        call_original( );
    }
};

CHEAT_HOOK_IMPL(should_skip_animation_frame);
