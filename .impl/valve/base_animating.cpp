#include <fd/valve/base_animating.h>

#if __has_include(<fd/netvars_generated/C_BaseAnimating_cpp_inc>)
#define NETVAR_CLASS base_animating
// ReSharper disable once CppUnusedIncludeDirective
#include <fd/netvars_generated/C_BaseAnimating_cpp_inc>
#endif

namespace fd::valve
{
#if __has_include(<fd/netvars_generated/C_BaseAnimating_cpp>)
#define NETVAR_CLASS base_animating
// ReSharper disable once CppUnusedIncludeDirective
#include <fd/netvars_generated/C_BaseAnimating_cpp>
#endif

void base_animating::UpdateClientSideAnimation()
{
    // 224
    // auto fn = fd::rt_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">();
    // fd::invoke((decltype(&base_animating::UpdateClientSideAnimation)&)fn, this);
}

void base_animating::InvalidateBoneCache()
{
    /*auto &time    = LastBoneSetupTime();
    auto &counter = MostRecentModelBoneCounter();

    time    = -FLT_MAX;
    counter = -1;*/
}
}