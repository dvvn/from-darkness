#pragma once
#include "interface_holder.h"
#include "functional/vfunc.h"
#include "render/backend/basic_dx9.h"

namespace fd
{
class system_library_info;
class native_dx9_backend;
FD_INTERFACE_FWD(native_dx9_backend, basic_dx9_backend, system_library_info);
} // namespace fd