module;

#include <fd/core/object.h>

module fd.csgo.interfaces.ClientState;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(CClientState*, 0, runtime_modules::engine.find_interface_sig<"A1 ? ? ? ? 8B 80 ? ? ? ? C3">().plus(1).deref<2>());
