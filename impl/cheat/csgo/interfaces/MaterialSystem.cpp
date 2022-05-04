module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.MaterialSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IMaterialSystem, csgo_modules::materialsystem.find_interface<"VMaterialSystem">( ));