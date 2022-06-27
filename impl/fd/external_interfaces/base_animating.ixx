module;

#include <cstdint>

export module fd.base_animating;
export import fd.csgo.interfaces.C_BaseEntity;
export import fd.csgo.interfaces.AnimationLayer;

class base_animating : public fd::csgo::C_BaseEntity
{
  public:
#if __has_include("C_BaseAnimating_generated_h")
#include "C_BaseAnimating_generated_h"
#endif

    void UpdateClientSideAnimation();
    void InvalidateBoneCache();
};

export namespace fd
{
    class QuaternionAligned;
    class CStudioHdr;
    class CBoneBitList;
    class CIKContext;

    using ::base_animating;

} // namespace fd
