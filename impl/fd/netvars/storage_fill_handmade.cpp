module;

#include <memory>

module fd.netvars.core:storage;
import :type_resolve;
// import fd.C_BaseAnimating;
import fd.rt_modules;
import fd.address;

namespace fd::math
{
    class matrix3x4;
}

using namespace fd;
using namespace netvars;

class C_BaseAnimating;
class base_entity;
struct CAnimationLayer;
struct VarMapping_t;

#define SIG(_MODULE_NAME_, _SIG_, ...)                                               \
    [] {                                                                             \
        return runtime_modules::_MODULE_NAME_.find_signature<_SIG_>().##__VA_ARGS__; \
    }

void storage::store_handmade_netvars()
{
    const auto baseent = this->find("base_entity");
    baseent->add<VarMapping_t>(0x24, "m_InterpVarMap");
    baseent->add<math::matrix3x4>(SIG(client, "8B 55 ? 85 D2 74 23 8B 87 ? ? ? ? 8B 4D ? 3B C8", plus(9).deref<1>().minus(8)), "m_BonesCache", type_utlvector);

    const auto baseanim = this->find("C_BaseAnimating");
    // m_vecRagdollVelocity - 128
    baseanim->add<CAnimationLayer>(SIG(client, "8B 87 ? ? ? ? 83 79 04 00 8B", plus(2).deref<1>()), "m_AnimOverlays", type_utlvector);
    // m_hLightingOrigin - 32
    baseanim->add<float>(SIG(client, "C7 87 ? ? ? ? ? ? ? ? 89 87 ? ? ? ? 8B 8F", plus(2).deref<1>()), " LastBoneSetupTime");
    // ForceBone + 4
    baseanim->add<int>(SIG(client, "89 87 ? ? ? ? 8B 8F ? ? ? ? 85 C9 74 10", plus(2).deref<1>()), "m_iMostRecentModelBoneCounter");

    //_Load_class<base_player>( );
}
