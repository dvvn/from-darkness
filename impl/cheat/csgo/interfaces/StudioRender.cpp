module cheat.csgo.interfaces.StudioRender;
import cheat.csgo.modules;

using namespace cheat;
using namespace csgo;

IStudioRender* nstd::one_instance_getter<IStudioRender*>::_Construct( )const
{
	return csgo_modules::studiorender->find_game_interface("VStudioRender");
}