#pragma once

#include <fd/valve/animation_layer.h>
#include <fd/valve/base_entity.h>


namespace fd::valve
{
    struct base_animating : base_entity
    {
#if __has_include("C_BaseAnimating_generated_h")
#include "C_BaseAnimating_generated_h"
#endif

        void UpdateClientSideAnimation();
        void InvalidateBoneCache();
    };
} // namespace fd::valve
