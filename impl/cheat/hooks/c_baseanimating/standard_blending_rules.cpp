module;

#include <cheat/hooks/hook.h>

module cheat.hooks.c_base_animating.standard_blending_rules;
import cheat.csgo.interfaces.C_BaseAnimating;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;
using namespace hooks;
using namespace c_base_animating;

CHEAT_HOOK(standard_blending_rules, member)
{
    standard_blending_rules_impl( )
    {
        const auto vtable_holder = csgo_modules::client.find_vtable<C_BaseAnimating>( );
        const auto index = csgo_modules::client.find_signature<"8D 94 ? ? ? ? ? 52 56 FF 90 ? ? ? ? 8B 47 FC">( ).plus(11).deref<1>( ).divide(4);

        this->init({vtable_holder, index}, &standard_blending_rules_impl::callback);
    }

    void callback(CStudioHdr * hdr, math::vector3 * pos, QuaternionAligned * q, float current_time, int bone_mask) const noexcept
    {
#if __has_include(<cheat/csgo/interfaces_custom/C_BaseAnimating_generated_h>) && 0
        const auto pl = reinterpret_cast<C_BaseAnimating*>(this);
        const auto client_class = pl->GetClientClass( );
        //if (client_class->ClassID != ClassId::CCSPlayer)
        //return;

        auto& flags = pl->m_fEffects( );
        const nstd::mem::backup flags_backup = flags;

        /*if (flags.has(m_fEffects_t::EF_NOINTERP))
            return;*/

        using namespace nstd::enum_operators;
        flags |= m_fEffects_t::EF_NOINTERP;
#endif
        return call_original(hdr, pos, q, current_time, bone_mask /*| BONE_USED_BY_HITBOX*/);

        /*if (override_return__)
            this->return_value_.store_value(override_return_to__);
        else
        {
            const auto pl = this->Target_instance( );
            const auto client_class = pl->GetClientClass( );
            if (client_class->ClassID != ClassId::CCSPlayer)
                return;

            const auto animate_this_frame = pl->m_bClientSideAnimation( );
            const auto skip_this_frame = animate_this_frame == false;
            this->return_value_.store_value(skip_this_frame);

            (void)client_class;
        }*/
    }
};

CHEAT_HOOK_IMPL(standard_blending_rules);