#include <fd/valve/cs_player.h>

#if __has_include("C_CSPlayer_generated_cpp")
#include "C_CSPlayer_generated_cpp"
#endif

namespace fd::valve
{
base_animating *cs_player::rag_doll()
{
#if __has_include("C_BaseAnimating_generated_h")
    return static_cast<base_animating *>(m_hRagdoll().Get());
#else
    return 0;
#endif
}
}