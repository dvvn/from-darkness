module;

#include <fd/core/object.h>

module fd.csgo.interfaces.GlobalVars;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(CGlobalVarsBase*, 0, runtime_modules::client.find_interface_sig<"A1 ? ? ? ? 5E 8B 40 10">().plus(1).deref<2>());
