module;

#include <cheat/tools/interface.h>

module cheat.csgo.interfaces.StudioRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_INTERFACE_IMPL(IStudioRender, csgo_modules::studiorender.find_interface<"VStudioRender">( ));