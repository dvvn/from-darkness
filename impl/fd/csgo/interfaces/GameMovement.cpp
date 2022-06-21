module;

#include <fd/core/object.h>

module fd.csgo.interfaces.GameMovement;
import fd.rt_modules;

using namespace fd;
using namespace csgo;

FD_OBJECT_IMPL(CGameMovement, 0, runtime_modules::client.find_interface<"GameMovement">());
