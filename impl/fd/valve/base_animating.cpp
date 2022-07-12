module;

#include <fd/assert.h>

#include <functional>

module fd.valve.base_animating;
import fd.rt_modules;

#if __has_include("C_BaseAnimating_generated_cpp")
#include "C_BaseAnimating_generated_cpp"
#endif

void base_animating::UpdateClientSideAnimation()
{
    // 224
    const decltype(&base_animating::UpdateClientSideAnimation) fn = fd::runtime_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">();
    std::invoke(fn, this);
}

void base_animating::InvalidateBoneCache()
{
#if __has_include("C_BaseAnimating_generated_cpp")
    auto& time    = LastBoneSetupTime();
    auto& counter = MostRecentModelBoneCounter();

    time    = -FLT_MAX;
    counter = -1;
#else
    FD_ASSERT_UNREACHABLE("Not implemented");
#endif
}
