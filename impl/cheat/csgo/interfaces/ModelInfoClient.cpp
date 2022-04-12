module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ModelInfoClient;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IVModelInfoClient)
{
	return csgo_modules::engine.find_interface<"VModelInfoClient">( );
}