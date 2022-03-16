module cheat.csgo.interfaces.ViewRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IViewRender* nstd::one_instance_getter<IViewRender*>::_Construct( )const
{
	return csgo_modules::client.find_signature<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">( ).plus(1).deref<1>( );
}