#pragma once

#include "tier2/core.h"

namespace FD_TIER2(native, cs2)
{
// InputSystemVersion001
class input_system
{
  public:
    bool is_relative_mouse_mode();
    void* get_SDL_window();
};
} // namespace FD_TIER2(native, cs2)
