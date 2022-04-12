module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ClientEntityList;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IClientEntityList)
{
	return csgo_modules::client.find_interface<"VClientEntityList">( );
}