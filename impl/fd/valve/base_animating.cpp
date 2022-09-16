module;

#include <fd/assert.h>

module fd.valve.base_animating;
import fd.functional.invoke;

#if __has_include("C_BaseAnimating_generated_cpp")
#include "C_BaseAnimating_generated_cpp"
#endif

void base_animating::UpdateClientSideAnimation()
{
    // 224
    // auto fn = fd::rt_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">();
    // fd::invoke((decltype(&base_animating::UpdateClientSideAnimation)&)fn, this);
    FD_ASSERT_UNREACHABLE("Not implemented");
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
