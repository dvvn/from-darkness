module;

#include <fd/utility.h>

#include <memory>

module fd.netvars.core:storage;
import :type_resolve;
import fd.valve.animation_layer;
import fd.valve.var_map;
import fd.math.matrix3x4;
import fd.rt_modules;
import fd.address;

#define _ADD_DOT(_X_) ._X_

#define SIG(_MODULE_NAME_, _SIG_, ...)                                                             \
    [] {                                                                                           \
        return rt_modules::_MODULE_NAME_.find_signature<_SIG_>()##FOR_EACH(_ADD_DOT, __VA_ARGS__); \
    }

void fd::netvars::storage::store_handmade_netvars()
{
    using namespace math;
    using namespace valve;

    const auto baseent = this->find("C_BaseEntity");
    baseent->add<var_map>(0x24, "m_InterpVarMap");
    baseent->add<matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9), deref<1>(), minus(8)), "m_BonesCache", type_utlvector);

    const auto baseanim = this->find("C_BaseAnimating");
    // m_vecRagdollVelocity - 128
    baseanim->add<animation_layer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2), deref<1>()), "m_AnimOverlays", type_utlvector);
    // m_hLightingOrigin - 32
    baseanim->add<float>(SIG(client, "C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F", plus(2), deref<1>()), "m_flLastBoneSetupTime");
    // ForceBone + 4
    baseanim->add<int>(SIG(client, "89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10", plus(2), deref<1>()), "m_iMostRecentModelBoneCounter");
}
