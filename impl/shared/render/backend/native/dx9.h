#pragma once
#include "interface_holder.h"
#include "functional/vfunc.h"
#include "render/backend/basic_dx9.h"

namespace fd
{
class system_library_info;
class dx9_backend_native;
FD_INTERFACE_FWD(dx9_backend_native, basic_dx9_backend, system_library_info);
} // namespace fd