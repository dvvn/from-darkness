module;

#include <fd/core/object.h>

module fd.csgo.interfaces.LocalPlayer;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(C_CSPlayer*, 0, runtime_modules::client.find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07">().plus(2).deref<1>());
