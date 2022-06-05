module;

#include <fds/hooks/hook.h>

module fds.hooks.c_base_entity.estimate_abs_velocity;
import fds.csgo.interfaces.C_BaseEntity;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;
using namespace hooks;
using namespace c_base_entity;

FDS_HOOK(estimate_abs_velocity, member){estimate_abs_velocity_impl(){const auto vtable = csgo_modules::client.find_vtable<C_BaseEntity>();
const auto index = csgo_modules::client.find_signature<"FF 90 ? ? 00 00 F3 0F 10 4C 24 18">().plus(2).deref<1>().divide(4);

this->init({vtable, index}, &estimate_abs_velocity_impl::callback);
}

void callback() const
{
    call_original();

#if 0
        using namespace nstd::enum_operators;

        const auto ent = this->get_object_instance( );

        if(ent->m_iEFlags( ) & m_iEFlags_t::EFL_DIRTY_ABSVELOCITY)
        {
            static auto calc_absolute_velocity = csgo_modules::client.find_signature<"55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F9 F7">( ).cast<void (C_BaseEntity::*)()>( );
            dhooks::call_function(calc_absolute_velocity, ent);
        }

        vel = ent->m_vecAbsVelocity( );

        this->store_return_value(true);
#endif
}
}
;

FDS_HOOK_IMPL(estimate_abs_velocity);
