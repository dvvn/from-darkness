module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.LocalPlayer;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(C_CSPlayer*, csgo_modules::client.find_interface_sig<"8B 0D ? ? ? ? 83 FF FF 74 07">().plus(2).deref<1>());
