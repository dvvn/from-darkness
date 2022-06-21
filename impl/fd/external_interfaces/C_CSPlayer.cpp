module;

#include <fd/core/assert.h>

module fd.csgo.interfaces.C_CSPlayer;
import fd.netvars;

using namespace fd::csgo;

#if __has_include("C_CSPlayer_generated_cpp")
#include "C_CSPlayer_generated_cpp"
#endif

C_BaseAnimating* C_CSPlayer::GetRagdoll()
{
#if __has_include("C_BaseAnimating_generated_h")
    return static_cast<C_BaseAnimating*>(m_hRagdoll().Get());
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
