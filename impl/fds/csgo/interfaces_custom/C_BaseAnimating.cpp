module;

#include <fds/core/assert.h>

#include <functional>

module fds.csgo.interfaces.C_BaseAnimating;
import fds.csgo.modules;
import fds.netvars;
import nstd.mem.address;

using namespace fds::csgo;

#if __has_include("C_BaseAnimating_generated_cpp")
#include "C_BaseAnimating_generated_cpp"
#endif

void C_BaseAnimating::UpdateClientSideAnimation()
{
    // 224
    static decltype(&C_BaseAnimating::UpdateClientSideAnimation) fn = csgo_modules::client.find_signature<"55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36">();
    std::invoke(fn, this);
}

void C_BaseAnimating::InvalidateBoneCache()
{
#if __has_include("C_BaseAnimating_generated_cpp")
    auto& time    = m_flLastBoneSetupTime();
    auto& counter = m_iMostRecentModelBoneCounter();

    time    = -FLT_MAX;
    counter = -1;
#else
    FDS_ASSERT("Not implemented");
#endif
}
