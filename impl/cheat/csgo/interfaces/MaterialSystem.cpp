module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.MaterialSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IMaterialSystem, csgo_modules::materialsystem.find_interface<"VMaterialSystem">( ));