module;

#include <cstdint>

export module fd.csgo.interfaces.C_BaseAnimating;
export import fd.csgo.interfaces.C_BaseEntity;
export import fd.csgo.interfaces.AnimationLayer;

export namespace fd::csgo
{
    class QuaternionAligned;
    class CStudioHdr;
    class CBoneBitList;
    class CIKContext;

    class C_BaseAnimating : public C_BaseEntity
    {
      public:
#if __has_include("C_BaseAnimating_generated_h")
#include "C_BaseAnimating_generated_h"
#endif

        void UpdateClientSideAnimation();
        void InvalidateBoneCache();
    };
} // namespace fd::csgo
