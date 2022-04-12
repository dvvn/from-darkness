module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.StudioRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IStudioRender)
{
	return csgo_modules::studiorender.find_interface<"VStudioRender">( );
}