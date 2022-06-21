export module fd.csgo.interfaces.C_CSPlayer;
export import fd.csgo.interfaces.C_BasePlayer;

export namespace fd::csgo
{
    ////econ
    // class CAttributeManager;
    // class CAttributeList;
    // class CAttributeContainer;

    class C_CSPlayer : public C_BasePlayer
    {
      public:
#if __has_include("C_CSPlayer_generated_h")
#include "C_CSPlayer_generated_h"
#endif

        C_BaseAnimating* GetRagdoll();
    };
} // namespace fd::csgo
