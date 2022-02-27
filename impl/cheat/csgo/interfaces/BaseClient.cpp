module;

module cheat.csgo.interfaces.BaseClient;
import cheat.csgo.modules;
import dhooks;

using namespace cheat;
using namespace csgo;

IBaseClientDLL* nstd::one_instance_getter<IBaseClientDLL*>::_Construct( )const
{
	return csgo_modules::client->find_game_interface("VClient");
}

bool IBaseClientDLL::DispatchUserMessage(int msg_type, int flags, int size, const void* msg)
{
	return dhooks::call_function(&IBaseClientDLL::DispatchUserMessage, this, 38, msg_type, flags, size, msg);
}
