module cheat.csgo.interfaces.WeaponSystem;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IWeaponSystem* nstd::one_instance_getter<IWeaponSystem*>::_Construct( )const
{
	return csgo_modules::client->find_signature("8B 35 ? ? ? ? FF 10 0F B7 C0").plus(2).deref<1>( );
}