#pragma once

#include "basic/winapi.h"
#include "hook/holder.h"

namespace fd
{
struct basic_win32_backend;

class hooked_wndproc;
FD_HOOK_FWD(hooked_wndproc, basic_winapi_hook, basic_win32_backend *);
} // namespace fd