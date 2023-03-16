#pragma once

#include <fd/valve/base_entity.h>

#if __has_include(<fd/netvars_generated/C_BaseAnimating_h_inc>)
#include <fd/netvars_generated/C_BaseAnimating_h_inc>
#endif

namespace fd::valve
{
struct base_animating : base_entity
{
#if __has_include(<fd/netvars_generated/C_BaseAnimating_h>)
#include <fd/netvars_generated/C_BaseAnimating_h>
#endif

    void UpdateClientSideAnimation();
    void InvalidateBoneCache();
};
} // namespace fd::valve