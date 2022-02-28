module;

module cheat.csgo.interfaces.LocalPlayer;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

C_CSPlayer** nstd::one_instance_getter<C_CSPlayer**>::_Construct( ) const
{
	return csgo_modules::client->find_signature("8B 0D ? ? ? ? 83 FF FF 74 07").plus(2).deref<1>( );
}
