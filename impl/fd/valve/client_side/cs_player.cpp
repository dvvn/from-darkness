#include <fd/valve/client_side/cs_player.h>
// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_CSPlayer_cpp_inc>)
#include <fd/valve/client_side/entity_list.h>
#define NETVAR_CLASS cs_player
#include <fd/netvars_generated/C_CSPlayer_cpp_inc>
#endif

namespace fd::valve::client_side
{
#if __has_include(<fd/netvars_generated/C_CSPlayer_cpp>)
#include <fd/netvars_generated/C_CSPlayer_cpp>
extern entity_list *ents_list;
#endif

// ReSharper disable CppMemberFunctionMayBeStatic

animating *cs_player::rag_doll()
{
#ifdef NETVAR_CLASS
    return static_cast<animating *>(m_hRagdoll().Get());
#else
    std::unreachable();
#endif
}
}