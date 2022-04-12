module;

#include <cheat/csgo/interface.h>

module cheat.csgo.interfaces.LocalPlayer;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

CHEAT_CSGO_INTERFACE_INIT(C_CSPlayer*)
{
	const auto ret = csgo_modules::client.find_signature<"8B 0D ? ? ? ? 83 FF FF 74 07">( ).plus(2).deref<1>( );
	csgo_modules::client.log_found_interface<C_CSPlayer*>(ret);
	return ret;
}
