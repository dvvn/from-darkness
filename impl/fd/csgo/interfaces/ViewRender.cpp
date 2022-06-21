module;

#include <fd/core/object.h>

module fd.csgo.interfaces.ViewRender;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IViewRender, 0, runtime_modules::client.find_interface_sig<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">().plus(1).deref<1>());
