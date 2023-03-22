#include <fd/valve/cs_player.h>

// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_CSPlayer_cpp_inc>)
#define NETVAR_CLASS cs_player
#include <fd/netvars_generated/C_CSPlayer_cpp_inc>
#endif

namespace fd::valve
{
#if __has_include(<fd/netvars_generated/C_CSPlayer_cpp>)
#include <fd/netvars_generated/C_CSPlayer_cpp>
#endif

base_animating *cs_player::rag_doll()
{
    return static_cast<base_animating *>(m_hRagdoll().Get());
}
}