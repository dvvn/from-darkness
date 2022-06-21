module;

#include <fd/core/object.h>

module fd.csgo.interfaces.MoveHelper;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IMoveHelper, 0, runtime_modules::client.find_interface_sig<"8B 0D ? ? ? ? 8B 45 ? 51 8B D4 89 02 8B 01">().plus(2).deref<2>());
