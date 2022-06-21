module;

#include <fd/core/object.h>

module fd.csgo.interfaces.ClientEntityList;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(IClientEntityList, 0, runtime_modules::client.find_interface<"VClientEntityList">());
