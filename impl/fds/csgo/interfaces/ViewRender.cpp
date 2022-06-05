module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.ViewRender;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IViewRender, csgo_modules::client.find_interface_sig<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">().plus(1).deref<1>());
