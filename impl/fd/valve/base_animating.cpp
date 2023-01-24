#include <fd/assert.h>
#include <fd/functional.h>
#include <fd/valve/base_animating.h>

#if __has_include("C_BaseAnimating_generated_cpp")
#include "C_BaseAnimating_generated_cpp"
#endif

using namespace fd;
using namespace valve;

void base_animating::UpdateClientSideAnimation()
{
    // 224
    // auto fn = fd::rt_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">();
    // fd::invoke((decltype(&base_animating::UpdateClientSideAnimation)&)fn, this);
    FD_ASSERT_PANIC("Not implemented");
}

void base_animating::InvalidateBoneCache()
{
#if __has_include("C_BaseAnimating_generated_cpp")
    auto& time    = LastBoneSetupTime();
    auto& counter = MostRecentModelBoneCounter();

    time    = -FLT_MAX;
    counter = -1;
#else
    FD_ASSERT_PANIC("Not implemented");
#endif
}
