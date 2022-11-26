#pragma once

#include <fd/valve/base_player.h>

namespace fd::valve
{
    struct cs_player : base_player
    {
#if __has_include("C_CSPlayer_generated_h")
#include "C_CSPlayer_generated_h"
#endif
        base_animating* rag_doll();
    };
} // namespace fd::valve
