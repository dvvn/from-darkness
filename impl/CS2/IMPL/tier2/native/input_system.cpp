#include "tier2/native/input_system.hpp"

namespace FD_TIER2(native, cs2)
{
bool input_system::is_relative_mouse_mode()
{
    // Offset in 'CInputSystem::SetRelativeMouseMode'.
    // Function is right above 'CInputSystem::DebugSpew'.

    return 0; // CPointer(this).GetField<bool>(0x4F);
}

void* input_system::get_SDL_window()
{
    // Offset in 'CInputSystem::DebugSpew'.
    // xref: "SDL clip window state on 0x%p is %s\n".

    // Offset in 'CInputSystem::SetCursorClip'.
    // xref: "SetCursorClip:  %s SDL_SetWindowGrab on 0x%p (%s) %s\n".

    return 0; // CPointer(this).GetField<void*>(platform::Constant(0x2678, 0x26D8));
}
}