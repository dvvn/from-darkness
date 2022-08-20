module;

#include <cstdint>

export module fd.valve.base_animating;
export import fd.valve.base_entity;
export import fd.valve.animation_layer;

using namespace fd::valve;

struct base_animating : base_entity
{
#if __has_include("C_BaseAnimating_generated_h")
#include "C_BaseAnimating_generated_h"
#endif

    void UpdateClientSideAnimation();
    void InvalidateBoneCache();
};

export namespace fd::valve
{
    using ::base_animating;
}
