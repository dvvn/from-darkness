module;

#include <cheat/hooks/hook.h>

module cheat.hooks.c_csplayer.do_extra_bone_processing;
import cheat.csgo.interfaces.C_CSPlayer;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_csplayer;

CHEAT_HOOK(do_extra_bone_processing, member)
{
    do_extra_bone_processing_impl( )
    {
        const auto vtable_holder = csgo_modules::client.find_vtable<C_CSPlayer>( );
        const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8D 4F FC">( ).plus(11).deref<1>( ).divide(4);

        this->init({vtable_holder, index}, &do_extra_bone_processing_impl::callback);
    }

    void callback(CStudioHdr * studio_hdr, math::vector3 * pos, math::quaternion * q, math::matrix3x4_aligned * bone_to_world, CBoneBitList & bone_computed, CIKContext * ik_context) const noexcept
    {
        //---
    }
};

CHEAT_HOOK_IMPL(do_extra_bone_processing)

