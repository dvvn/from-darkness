module;

#include <fd/core/object.h>

module fd.csgo.interfaces.ClientMode;
import fd.csgo.interfaces.BaseClient;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(ClientModeShared, 0, runtime_modules::client._Ifc_finder(&FD_OBJECT_GET(IBaseClientDLL)).deref<1>()[10].plus(5).deref<2>());
