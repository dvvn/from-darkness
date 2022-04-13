module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.ViewRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(IViewRender)
{
	return csgo_modules::client.find_interface_sig<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">( ).plus(1).deref<1>( );
}