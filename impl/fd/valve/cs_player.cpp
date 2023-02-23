#include <fd/valve/cs_player.h>

#if __has_include("C_CSPlayer_generated_cpp")
// import fd.netvars;
// using C_CSPlayer = cs_player;
#include "C_CSPlayer_generated_cpp"
#endif

using namespace fd::valve;

base_animating* cs_player::rag_doll()
{
#if __has_include("C_BaseAnimating_generated_h")
    return static_cast<base_animating*>(m_hRagdoll().Get());
#else
    return 0;
#endif
}