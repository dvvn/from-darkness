module;

#include <fd/core/assert.h>

module fd.cs_player;

#if __has_include("C_CSPlayer_generated_cpp")
import fd.netvars;
using C_CSPlayer = cs_player;
#include "C_CSPlayer_generated_cpp"
#endif

fd::base_animating* cs_player::rag_doll()
{
#if __has_include("C_BaseAnimating_generated_h")
    return static_cast<fd::base_animating*>(m_hRagdoll().Get());
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
