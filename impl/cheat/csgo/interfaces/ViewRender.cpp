module cheat.csgo.interfaces.ViewRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IViewRender* nstd::one_instance_getter<IViewRender*>::_Construct( )const
{
	IViewRender* const ret = csgo_modules::client.find_signature<"A1 ? ? ? ? B9 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? FF 10">( ).plus(1).deref<1>( );
	csgo_modules::client.log_found_interface(ret);
	return ret;
}