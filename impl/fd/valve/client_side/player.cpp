#include <fd/valve/client_side/player.h>

// ReSharper disable CppUnusedIncludeDirective

#if __has_include(<fd/netvars_generated/C_BasePlayer_cpp_inc>)
#define NETVAR_CLASS player
#include <fd/netvars_generated/C_BasePlayer_cpp_inc>
#endif

namespace fd::valve::client_side
{
#if __has_include(<fd/netvars_generated/C_BasePlayer_cpp>)
#include <fd/netvars_generated/C_BasePlayer_cpp>
#endif
}