module;

#include <fd/object.h>

module fd.valve.client_mode;
import fd.valve.base_client;
import fd.rt_modules;

using namespace fd::valve;

// FD_OBJECT_IMPL(client_mode_shared, fd::rt_modules::client._Ifc_finder(&FD_OBJECT_GET(base_client)).deref<1>()[10].plus(5).deref<2>());
