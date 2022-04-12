module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.MDLCache;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IMDLCache)
{
	return csgo_modules::datacache.find_interface<"MDLCache">( );
}