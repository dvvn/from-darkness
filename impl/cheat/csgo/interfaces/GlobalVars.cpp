module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.GlobalVars;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(CGlobalVarsBase)
{
	return csgo_modules::client.find_interface_sig<"A1 ? ? ? ? 5E 8B 40 10">( ).plus(1).deref<2>( );
}