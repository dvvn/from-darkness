module;

#include <fds/tools/interface.h>

module fds.csgo.interfaces.MaterialSystem;
import fds.csgo.modules;

using namespace fds;
using namespace csgo;

FDS_INTERFACE_IMPL(IMaterialSystem, csgo_modules::materialsystem.find_interface<"VMaterialSystem">());
