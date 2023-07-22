#pragma once
#include "basic/directx9.h"
#include "hook/holder.h"

namespace fd
{
struct basic_render_backend;
struct basic_render_frame;

class hooked_directx9_reset;
FD_HOOK_FWD(hooked_directx9_reset, basic_directx9_hook, basic_render_backend *);
class hooked_directx9_present;
FD_HOOK_FWD(hooked_directx9_present, basic_directx9_hook, basic_render_frame *);
}